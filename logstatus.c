/*
 * (C) 2008 by Tomasz bla Fortuna
 *
 * DESC: Pidgin status logging.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * any later version.
 *
 *
 */

#define GETTEXT_PACKAGE "plugin_pack"
#define PURPLE_PLUGINS

#include <string.h>
#include <glib.h>
#include <locale.h>
#include <libintl.h>

#include <glib/gi18n-lib.h>

#include <libpurple/conversation.h>
#include <libpurple/signals.h>

#include <libpurple/plugin.h>
#include <libpurple/version.h>

#define LOGSTATUS_PLUGIN_ID "core-logstatus"

static void
write_message(PurpleBuddy *buddy, const char *message)
{
	PurpleConversation *conv;
	const char *alias = purple_buddy_get_alias(buddy);

	/* Acquire conversion */
	conv = purple_find_conversation_with_account(
		PURPLE_CONV_TYPE_ANY, buddy->name, buddy->account);
	
	if (conv != NULL) {
		/* If conversation is open just place there the message using purple_conversation_write() */
/*		printf("Conv logging! Conv name = %s\n", conv->name); */
		purple_conversation_write(conv, alias, message, PURPLE_MESSAGE_SYSTEM, time(NULL));
	} else {
		/* When it's not - open an log */
		/* TODO: Make it append data to the most recent log */
		PurpleLog *log;

		log = purple_log_new(PURPLE_LOG_IM, buddy->name, buddy->account, NULL, time(NULL), NULL);
/*		printf("Plain log logging! buddy->name = %s mesg = %s\n", buddy->name, message); */

		if (!log) {
			printf("logstatus plugin: Log creation failed!\n");
			return;
		}
		purple_log_write(log, PURPLE_MESSAGE_SYSTEM, alias, time(NULL), message);
		purple_log_free(log);
	}
	return;
}


static void
write_status(PurpleBuddy *buddy, const char *fmt)
{
	const char *message;
	const char *status_type;
	const char *alias = purple_buddy_get_alias(buddy);
	char *log_entry;

	PurpleStatus *status = purple_presence_get_active_status(buddy->presence);

	/* Collect data */
	message = purple_status_get_attr_string(status, "message");
	if (message == NULL) {
		/* Don't bother with no message only
		 * if we have something other to say */
		if (fmt) 
			message = _("");
		else
			return;
	}
	
	if (!fmt) {
		/* No fmt? Use default */
		fmt = _("%s status is: %s (%s)");	
	}

	if (!status) {
		status_type = _("Unknown status type");
	} else {
		status_type = purple_status_get_name(status);
	}

	log_entry = g_strdup_printf(fmt, alias, status_type, message);
	write_message(buddy, log_entry);
	g_free(log_entry);
	return;
}


static void
buddy_status_changed_cb(PurpleBuddy *buddy, PurpleStatus *old_status,
                        PurpleStatus *status, void *data)
{
	g_return_if_fail(buddy != NULL);
	g_return_if_fail(status != NULL);
	write_status(buddy, NULL);
}

static void
buddy_idle_changed_cb(PurpleBuddy *buddy, gboolean old_idle, gboolean idle,
                      void *data)
{
	if (idle) {
		write_status(buddy, _("%s has become idle: %s (%s)"));
	} else {
		write_status(buddy, _("%s is no longer idle: %s (%s)"));
	}
}

static void
buddy_signon_cb(PurpleBuddy *buddy, void *data)
{
	write_status(buddy, _("%s has signon: %s (%s)"));
}

static void
buddy_signoff_cb(PurpleBuddy *buddy, void *data)
{
	write_status(buddy, _("%s has signoff: %s (%s)"));
}

static gboolean
plugin_load(PurplePlugin *plugin)
{
	void *blist_handle = purple_blist_get_handle();

	purple_signal_connect(blist_handle, "buddy-status-changed", plugin,
		PURPLE_CALLBACK(buddy_status_changed_cb), NULL);
	purple_signal_connect(blist_handle, "buddy-idle-changed", plugin,
		PURPLE_CALLBACK(buddy_idle_changed_cb), NULL);
	purple_signal_connect(blist_handle, "buddy-signed-on", plugin,
		PURPLE_CALLBACK(buddy_signon_cb), NULL);
	purple_signal_connect(blist_handle, "buddy-signed-off", plugin,
		PURPLE_CALLBACK(buddy_signoff_cb), NULL);
	return TRUE;
}

static PurplePluginUiInfo prefs_info =
{
	NULL,
	0,
	NULL,

	/* padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static PurplePluginInfo info =
{
	PURPLE_PLUGIN_MAGIC,
	PURPLE_MAJOR_VERSION,
	PURPLE_MINOR_VERSION,
	PURPLE_PLUGIN_STANDARD,
	NULL,
	0,
	NULL,
	PURPLE_PRIORITY_DEFAULT,

	LOGSTATUS_PLUGIN_ID,
	N_("Buddy Status history"),
	N_("0.5v"),

	N_("Stores in the buddy history his status changes"),
	N_("Stores in the buddy history his status changes"),
	" Tomasz bla Fortuna <bla@thera.be>",
	N_("http://bla.thera.be"),

	plugin_load,
	NULL,
	NULL,

	NULL,
	NULL,
	&prefs_info,
	NULL,

	/* padding */
	NULL,
	NULL,
	NULL,
	NULL
};

static void
init_plugin(PurplePlugin *plugin)
{
	printf("Init plugin!!!\n");
}

PURPLE_INIT_PLUGIN(logstatus, init_plugin, info)
