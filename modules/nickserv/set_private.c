/*
 * Copyright (c) 2006-2007 William Pitcock, et al.
 * Rights to this code are documented in doc/LICENSE.
 *
 * This file contains routines to handle the NickServ SET PRIVATE command.
 */

#include "atheme.h"
#include "list_common.h"
#include "list.h"

SIMPLE_DECLARE_MODULE_V1("nickserv/set_private", MODULE_UNLOAD_CAPABILITY_OK,
                         _modinit, _moddeinit);

mowgli_patricia_t **ns_set_cmdtree;

/* SET PRIVATE ON|OFF */
static void ns_cmd_set_private(sourceinfo_t *si, int parc, char *parv[])
{
	char *params = parv[0];

	if (!params)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "PRIVATE");
		return;
	}

	if (!strcasecmp("ON", params))
	{
		if (MU_PRIVATE & si->smu->flags)
		{
			command_fail(si, fault_nochange, _("The \2%s\2 flag is already set for \2%s\2."), "PRIVATE", entity(si->smu)->name);
			return;
		}

		logcommand(si, CMDLOG_SET, "SET:PRIVATE:ON");

		si->smu->flags |= MU_PRIVATE;
		si->smu->flags |= MU_HIDEMAIL;

		command_success_nodata(si, _("The \2%s\2 flag has been set for \2%s\2."), "PRIVATE" ,entity(si->smu)->name);

		return;
	}
	else if (!strcasecmp("OFF", params))
	{
		if (!(MU_PRIVATE & si->smu->flags))
		{
			command_fail(si, fault_nochange, _("The \2%s\2 flag is not set for \2%s\2."), "PRIVATE", entity(si->smu)->name);
			return;
		}

		logcommand(si, CMDLOG_SET, "SET:PRIVATE:OFF");

		si->smu->flags &= ~MU_PRIVATE;

		command_success_nodata(si, _("The \2%s\2 flag has been removed for \2%s\2."), "PRIVATE", entity(si->smu)->name);

		return;
	}
	else
	{
		command_fail(si, fault_badparams, STR_INVALID_PARAMS, "PRIVATE");
		return;
	}
}

command_t ns_set_private = { "PRIVATE", N_("Hides information about you from other users."), AC_NONE, 1, ns_cmd_set_private, { .path = "nickserv/set_private" } };

static bool has_private(const mynick_t *mn, const void *arg)
{
	myuser_t *mu = mn->owner;

	return ( mu->flags & MU_PRIVATE ) == MU_PRIVATE;
}

void _modinit(module_t *m)
{
	MODULE_TRY_REQUEST_SYMBOL(m, ns_set_cmdtree, "nickserv/set_core", "ns_set_cmdtree");
	command_add(&ns_set_private, *ns_set_cmdtree);

	use_nslist_main_symbols(m);

	static list_param_t private;
	private.opttype = OPT_BOOL;
	private.is_match = has_private;

	list_register("private", &private);

	use_account_private++;
}

void _moddeinit(module_unload_intent_t intent)
{
	command_delete(&ns_set_private, *ns_set_cmdtree);

	list_unregister("private");

	use_account_private--;
}
