/*
 * Copyright (C) 2021 Tobias Brunner, codelabs GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#define _GNU_SOURCE
#include <stdio.h>
#ifdef USE_SELINUX
#include <selinux/selinux.h>
#endif

#include "sec_label.h"

typedef struct private_sec_label_t private_sec_label_t;

/**
 * Private data.
 */
struct private_sec_label_t {

	/**
	 * Public interface
	 */
	sec_label_t public;

	/**
	 * Encoded label value
	 */
	chunk_t encoding;

	/**
	 * String representation of the label
	 */
	char *str;
};

static sec_label_t *create_sec_label(chunk_t encoding, char *str);

METHOD(sec_label_t, get_encoding, chunk_t,
	private_sec_label_t *this)
{
	return this->encoding;
}

METHOD(sec_label_t, get_string, char*,
	private_sec_label_t *this)
{
	return this->str;
}

METHOD(sec_label_t, clone_, sec_label_t*,
	private_sec_label_t *this)
{
	return create_sec_label(chunk_clone(this->encoding), strdup(this->str));
}

METHOD(sec_label_t, equals, bool,
	private_sec_label_t *this, sec_label_t *other_pub)
{
	private_sec_label_t *other = (private_sec_label_t*)other_pub;

	if (!other_pub)
	{
		return FALSE;
	}
	return chunk_equals_const(this->encoding, other->encoding);
}

METHOD(sec_label_t, matches, bool,
	private_sec_label_t *this, sec_label_t *other_pub)
{
	if (!other_pub)
	{
		return FALSE;
	}
#ifdef USE_SELINUX
	private_sec_label_t *other = (private_sec_label_t*)other_pub;
	return selinux_check_access(other->str, this->str, "association",
								"polmatch", NULL) == 0;
#else
	return equals(this, other_pub);
#endif
}

METHOD(sec_label_t, hash, u_int,
	private_sec_label_t *this, u_int inc)
{
	return chunk_hash_inc(this->encoding, inc);
}

METHOD(sec_label_t, destroy, void,
	private_sec_label_t *this)
{
	chunk_free(&this->encoding);
	free(this->str);
	free(this);
}

/**
 * Internal constructor, data is adopted
 */
static sec_label_t *create_sec_label(chunk_t encoding, char *str)
{
	private_sec_label_t *this;

	INIT(this,
		.public = {
			.get_encoding = _get_encoding,
			.get_string = _get_string,
			.clone = _clone_,
			.matches = _matches,
			.equals = _equals,
			.hash = _hash,
			.destroy = _destroy,
		},
		.encoding = encoding,
		.str = str,
	);
	return &this->public;
}

/*
 * Described in header
 */
sec_label_t *sec_label_from_encoding(const chunk_t value)
{
	chunk_t sanitized = chunk_empty;
	char *str;

	if (!value.len)
	{
		DBG1(DBG_LIB, "invalid empty security label");
		return NULL;
	}

	if (!chunk_printable(value, &sanitized, '?'))
	{
#ifdef USE_SELINUX
		/* don't accept labels with non-printable characters if we use SELinux */
		DBG1(DBG_LIB, "invalid security label with non-printable characters %B",
			 &value);
		chunk_free(&sanitized);
		return NULL;
#endif
	}
	if (asprintf(&str, "%.*s", (int)sanitized.len, sanitized.ptr) <= 0)
	{
		chunk_free(&sanitized);
		return NULL;
	}
	chunk_free(&sanitized);

	return create_sec_label(chunk_clone(value), str);
}

/*
 * Described in header
 */
sec_label_t *sec_label_from_string(const char *value)
{
	if (!value)
	{
		return NULL;
	}
	return sec_label_from_encoding(chunk_from_str((char*)value));
}
