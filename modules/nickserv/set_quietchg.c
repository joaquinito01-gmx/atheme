/*
 * Copyright (c) 2005 William Pitcock <nenolod -at- nenolod.net>
 * Copyright (c) 2007 Jilles Tjoelker
 * Rights to this code are as documented in doc/LICENSE.
 *
 * Allows you to opt-out of channel change messages.
 */

#include "atheme.h"
#include "uplink.h"
#include "list_common.h"
#include "list.h"

SIMPLE_DECLARE_MODULE_V1("nickserv/set_quietchg", MODULE_UNLOAD_CAPABILITY_OK,
                         _modinit, _moddeinit);

mowgli_patricia_t **ns_set_cmdtree;

static void ns_cmd_set_quietchg(sourceinfo_t *si, int parc, char *parv[]);

command_t ns_set_quietchg = { "QUIETCHG", N_("Allows you to opt-out of channel change messages."), AC_NONE, 1, ns_cmd_set_quietchg, { .path = "nickserv/set_quietchg" } };

static bool has_quietchg(const mynick_t *mn, const void *arg)
{
	myuser_t *mu = mn->owner;

	return ( mu->flags & MU_QUIETCHG ) == MU_QUIETCHG;
}

void _modinit(module_t *m)
{
	MODULE_TRY_REQUEST_SYMBOL(m, ns_set_cmdtree, "nickserv/set_core", "ns_set_cmdtree");

	command_add(&ns_set_quietchg, *ns_set_cmdtree);

	use_nslist_main_symbols(m);

	static list_param_t quietchg;
	quietchg.opttype = OPT_BOOL;
	quietchg.is_match = has_quietchg;

	list_register("quietchg", &quietchg);
}

void _moddeinit(module_unload_intent_t intent)
{
	command_delete(&ns_set_quietchg, *ns_set_cmdtree);

	list_unregister("quietchg");
}

/* SET QUIETCHG [ON|OFF] */
static void ns_cmd_set_quietchg(sourceinfo_t *si, int parc, char *parv[])
{
	char *setting = parv[0];

	if (!setting)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "QUIETCHG");
		return;
	}

	if (!strcasecmp("ON", setting))
	{
		if (MU_QUIETCHG & si->smu->flags)
		{
			command_fail(si, fault_nochange, _("The \2%s\2 flag is already set for account \2%s\2."), "QUIETCHG", entity(si->smu)->name);
			return;
		}

		logcommand(si, CMDLOG_SET, "SET:QUIETCHG:ON");

		si->smu->flags |= MU_QUIETCHG;

		command_success_nodata(si, _("The \2%s\2 flag has been set for account \2%s\2."), "QUIETCHG" ,entity(si->smu)->name);

		return;
	}
	else if (!strcasecmp("OFF", setting))
	{
		if (!(MU_QUIETCHG & si->smu->flags))
		{
			command_fail(si, fault_nochange, _("The \2%s\2 flag is not set for account \2%s\2."), "QUIETCHG", entity(si->smu)->name);
			return;
		}

		logcommand(si, CMDLOG_SET, "SET:QUIETCHG:OFF");

		si->smu->flags &= ~MU_QUIETCHG;

		command_success_nodata(si, _("The \2%s\2 flag has been removed for account \2%s\2."), "QUIETCHG", entity(si->smu)->name);

		return;
	}
	else
	{
		command_fail(si, fault_badparams, STR_INVALID_PARAMS, "QUIETCHG");
		return;
	}
}
