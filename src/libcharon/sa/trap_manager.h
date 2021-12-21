/*
 * Copyright (C) 2013-2017 Tobias Brunner
 * Copyright (C) 2009 Martin Willi
 * HSR Hochschule fuer Technik Rapperswil
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

/**
 * @defgroup trap_manager trap_manager
 * @{ @ingroup sa
 */

#ifndef TRAP_MANAGER_H_
#define TRAP_MANAGER_H_

#include <library.h>
#include <collections/enumerator.h>
#include <config/peer_cfg.h>
#include <sa/child_sa.h>

typedef struct trap_manager_t trap_manager_t;

/**
 * Manage policies to create SAs from traffic.
 */
struct trap_manager_t {

	/**
	 * Install a policy as a trap.
	 *
	 * @param peer		peer configuration to initiate on trap
	 * @param child 	child configuration to install as a trap
	 * @return			TRUE if successfully installed
	 */
	bool (*install)(trap_manager_t *this, peer_cfg_t *peer, child_cfg_t *child);

	/**
	 * Uninstall a trap policy.
	 *
	 * If no peer configuration name is given the first matching child
	 * configuration is uninstalled.
	 *
	 * @param peer		peer configuration name or NULL
	 * @param child		child configuration name
	 * @return			TRUE if uninstalled successfully
	 */
	bool (*uninstall)(trap_manager_t *this, char *peer, char *child);

	/**
	 * Register a CHILD_SA for which acquires might get triggered.
	 *
	 * This could be used for CHILD_SAs whose policies might trigger acquires
	 * but for which it is not possible to install trap policies (e.g. because
	 * they are for roadwarriors but due to security labels acquires might get
	 * triggered).
	 *
	 * @param peer		peer configuration to register
	 * @param child 	CHILD_SA to register
	 * @return			TRUE if successfully registered
	 */
	bool (*add_child)(trap_manager_t *this, peer_cfg_t *peer, child_sa_t *child);

	/**
	 * Remove a previously registered CHILD_SA.
	 *
	 * @param child 	CHILD_SA to remove
	 * @return			TRUE if successfully removed
	 */
	bool (*remove_child)(trap_manager_t *this, child_sa_t *child);

	/**
	 * Create an enumerator over all installed traps (does not include
	 * registered CHILD_SAs).
	 *
	 * @return			enumerator over (peer_cfg_t, child_sa_t)
	 */
	enumerator_t* (*create_enumerator)(trap_manager_t *this);

	/**
	 * Acquire an SA triggered by an installed trap.
	 *
	 * @param reqid		reqid of the triggered policy
	 * @param data		data from the acquire
	 */
	void (*acquire)(trap_manager_t *this, uint32_t reqid,
					kernel_acquire_data_t *data);

	/**
	 * Clear any installed trap.
	 */
	void (*flush)(trap_manager_t *this);

	/**
	 * Destroy a trap_manager_t.
	 */
	void (*destroy)(trap_manager_t *this);
};

/**
 * Create a trap_manager instance.
 */
trap_manager_t *trap_manager_create();

#endif /** TRAP_MANAGER_H_ @}*/
