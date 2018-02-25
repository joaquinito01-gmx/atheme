/*
 * Copyright (c) 2003-2004 E. Will et al.
 * Copyright (c) 2006-2010 Atheme Development Group
 * Rights to this code are documented in doc/LICENSE.
 *
 * This file contains routines to handle the CService SET ENTRYMSG command.
 */

#include "atheme.h"

static void cs_cmd_set_entrymsg(struct sourceinfo *si, int parc, char *parv[]);

static struct command cs_set_entrymsg = { "ENTRYMSG", N_("Sets the channel's entry message."), AC_NONE, 2, cs_cmd_set_entrymsg, { .path = "cservice/set_entrymsg" } };

static mowgli_patricia_t **cs_set_cmdtree = NULL;

static void
mod_init(struct module *const restrict m)
{
	MODULE_TRY_REQUEST_SYMBOL(m, cs_set_cmdtree, "chanserv/set_core", "cs_set_cmdtree");

	command_add(&cs_set_entrymsg, *cs_set_cmdtree);
}

static void
mod_deinit(const enum module_unload_intent ATHEME_VATTR_UNUSED intent)
{
	command_delete(&cs_set_entrymsg, *cs_set_cmdtree);
}

static void
cs_cmd_set_entrymsg(struct sourceinfo *si, int parc, char *parv[])
{
	struct mychan *mc;

	if (!(mc = mychan_find(parv[0])))
	{
		command_fail(si, fault_nosuch_target, _("Channel \2%s\2 is not registered."), parv[0]);
		return;
	}

	if (!chanacs_source_has_flag(mc, si, CA_SET))
	{
		command_fail(si, fault_noprivs, _("You are not authorized to execute this command."));
		return;
	}

	if (!parv[1] || !strcasecmp("OFF", parv[1]) || !strcasecmp("NONE", parv[1]))
	{
		/* entrymsg is private because users won't see it if they're AKICKED,
		 * if the channel is +i, or if the channel is RESTRICTED
		 */
		if (metadata_find(mc, "private:entrymsg"))
		{
			metadata_delete(mc, "private:entrymsg");
			logcommand(si, CMDLOG_SET, "SET:ENTRYMSG:NONE: \2%s\2", mc->name);
			verbose(mc, _("\2%s\2 cleared the entry message"), get_source_name(si));
			command_success_nodata(si, _("The entry message for \2%s\2 has been cleared."), parv[0]);
			return;
		}

		command_fail(si, fault_nochange, _("The entry message for \2%s\2 was not set."), parv[0]);
		return;
	}

	/* we'll overwrite any existing metadata.
	 * Why is/was this even private? There are no size/content sanity checks
	 * and even users with no channel access can see it. --jdhore
	 */
	metadata_add(mc, "private:entrymsg", parv[1]);

	logcommand(si, CMDLOG_SET, "SET:ENTRYMSG: \2%s\2 \2%s\2", mc->name, parv[1]);
	verbose(mc, _("\2%s\2 set the entry message for the channel to \2%s\2"), get_source_name(si), parv[1]);
	command_success_nodata(si, _("The entry message for \2%s\2 has been set to \2%s\2"), parv[0], parv[1]);
}

SIMPLE_DECLARE_MODULE_V1("chanserv/set_entrymsg", MODULE_UNLOAD_CAPABILITY_OK)
