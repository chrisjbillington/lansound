/* paswitch, PulseAudio commandline sink switcher
 * Copyright (C) 2012  Tomaz Solc <tomaz.solc@tablix.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */
#include <stdio.h>

#include <pulse/pulseaudio.h>
#include <pulse/ext-stream-restore.h>

pa_mainloop* mainloop;
pa_mainloop_api* mainloop_api;
pa_context* context;

static int setup_context()
{
	mainloop = pa_mainloop_new();
	if (!mainloop) {
        	printf("pa_mainloop_new() failed.\n");
		return -1;
	}

	mainloop_api = pa_mainloop_get_api(mainloop);

        pa_proplist     *proplist;

        proplist = pa_proplist_new();
        pa_proplist_sets(proplist,
                          PA_PROP_APPLICATION_NAME,
                          "Commandline sink switcher");
        pa_proplist_sets(proplist,
                          PA_PROP_APPLICATION_ID,
                          "org.tablix.paswitch");

	context = pa_context_new_with_proplist(
			mainloop_api, NULL, proplist);
    	if (!context) {
        	printf("pa_context_new() failed.\n");
		return -1;
    	}

        pa_proplist_free(proplist);

    	if(pa_context_connect(context, NULL, 0, NULL) < 0) {
        	printf("pa_context_connect() failed: %s\n", 
				pa_strerror(pa_context_errno(context)));
		return -1;
	}

	return 0;
}

static void context_drain_complete(pa_context *c, void *userdata)
{
    pa_context_disconnect(c);
}

static void drain(void)
{
    pa_operation *o;

    if (!(o = pa_context_drain(context, context_drain_complete, NULL)))
        pa_context_disconnect(context);
    else
        pa_operation_unref(o);
}

static void success_cb(pa_context* c, int success, void *userdata) 
{
	if(!success) {
                printf("%s\n", pa_strerror(pa_context_errno(c)));
    		mainloop_api->quit(mainloop_api, 1);
	}
}

static void stream_restore_cb(pa_context *c,
		const pa_ext_stream_restore_info *info, int eol, void *userdata)
{
	char* name = (char*) userdata;

        pa_ext_stream_restore_info new_info;

        if(eol) {
		drain();
		return;
	}

        new_info.name = info->name;
        new_info.channel_map = info->channel_map;
        new_info.volume = info->volume;
        new_info.mute = info->mute;

        new_info.device = name;

        pa_operation *o;
        o = pa_ext_stream_restore_write (context,
                                         PA_UPDATE_REPLACE,
                                         &new_info, 1,
                                         1, success_cb, NULL);
        if(o == NULL) {
                printf("pa_ext_stream_restore_write() failed: %s\n",
                           pa_strerror(pa_context_errno(context)));
                return;
        }

        //printf("Changed default device for %s to %s\n", info->name, info->device);

        pa_operation_unref (o);
}

static int set_default_sink(char* name)
{
        pa_operation *o;

        o = pa_context_set_default_sink(context, name, success_cb, NULL);
        if(o == NULL) {
		printf("pa_context_set_default_sink() failed: %s\n",
                           pa_strerror(pa_context_errno(context)));
		return -1;
        }

        pa_operation_unref(o);

        o = pa_ext_stream_restore_read(context,
                                        stream_restore_cb,
					name);

        if(o == NULL) {
                printf("pa_ext_stream_restore_read() failed: %s\n",
                           pa_strerror(pa_context_errno(context)));
                return -1;
        }

        pa_operation_unref(o);

        return 0;
}

static void context_state_callback(pa_context *c, void *userdata)
{
	char* name = (char*) userdata;

	switch(pa_context_get_state(c)) {
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
			break;

		case PA_CONTEXT_READY:
			//printf("setting default sink to %s\n", name);

			if(set_default_sink(name)) {
				printf("set_default_sink() failed\n");
    				mainloop_api->quit(mainloop_api, 1);
			}

			break;

		case PA_CONTEXT_TERMINATED:
    			mainloop_api->quit(mainloop_api, 0);
			break;

		default:
			printf("connection failure: %s\n",
					pa_strerror(pa_context_errno(c)));
			mainloop_api->quit(mainloop_api, 1);
	}
}

int main(int argc, char** argv) 
{
	if(argc != 2) {
		printf(
			"PulseAudio commandline sink switcher\n"
			"Copyright (C) 2012 by Tomaz Solc <tomaz.solc@tablix.org>\n\n"
			"USAGE: %s [ sink ]\n",
			argv[0]);

		return 0;
	}

	if(setup_context()) {
		printf("can't get pulseaudio context.\n");
		return 1;
	}

	char* name = argv[1];
	pa_context_set_state_callback(context, context_state_callback, name);

	int ret;
	if(pa_mainloop_run(mainloop, &ret) < 0) {
		printf("pa_mainloop_run() failed.\n");
		return 1;
	}

	pa_context_disconnect(context);
        pa_context_unref(context);
        pa_signal_done();
        pa_mainloop_free(mainloop);

	return ret;
}
