/*
This file is part of the Stargate project, Copyright Stargate Team

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "plugin.h"
#include "audiodsp/lib/amp.h"
#include "audiodsp/modules/filter/svf.h"
#include "audiodsp/modules/delay/reverb.h"
#include "plugins/multifx.h"

int MULTIFX_AMORITIZER = 0;

void v_multifx_cleanup(PluginHandle instance)
{
    free(instance);
}

void v_multifx_set_cc_map(PluginHandle instance, char * a_msg)
{
    t_multifx *plugin = (t_multifx *)instance;
    v_generic_cc_map_set(&plugin->cc_map, a_msg);
}

void v_multifx_panic(PluginHandle instance)
{
    //t_multifx *plugin = (t_multifx*)instance;
}

void v_multifx_on_stop(PluginHandle instance)
{
    //t_multifx *plugin = (t_multifx*)instance;
}

void v_multifx_connect_buffer(PluginHandle instance, int a_index,
        SGFLT * DataLocation, int a_is_sidechain)
{
    if(a_is_sidechain)
    {
        return;
    }
    t_multifx *plugin = (t_multifx*)instance;

    switch(a_index)
    {
        case 0:
            plugin->output0 = DataLocation;
            break;
        case 1:
            plugin->output1 = DataLocation;
            break;
        default:
            assert(0);
            break;
    }
}

void v_multifx_connect_port(PluginHandle instance, int port,
        PluginData * data)
{
    t_multifx *plugin;

    plugin = (t_multifx *) instance;

    switch (port)
    {
        case MULTIFX_FX0_KNOB0: plugin->fx_knob0[0] = data; break;
        case MULTIFX_FX0_KNOB1:        plugin->fx_knob1[0] = data; break;
        case MULTIFX_FX0_KNOB2: plugin->fx_knob2[0] = data; break;
        case MULTIFX_FX0_COMBOBOX: plugin->fx_combobox[0] = data; break;

        case MULTIFX_FX1_KNOB0: plugin->fx_knob0[1] = data; break;
        case MULTIFX_FX1_KNOB1:        plugin->fx_knob1[1] = data; break;
        case MULTIFX_FX1_KNOB2: plugin->fx_knob2[1] = data; break;
        case MULTIFX_FX1_COMBOBOX: plugin->fx_combobox[1] = data; break;

        case MULTIFX_FX2_KNOB0: plugin->fx_knob0[2] = data; break;
        case MULTIFX_FX2_KNOB1:        plugin->fx_knob1[2] = data; break;
        case MULTIFX_FX2_KNOB2: plugin->fx_knob2[2] = data; break;
        case MULTIFX_FX2_COMBOBOX: plugin->fx_combobox[2] = data; break;

        case MULTIFX_FX3_KNOB0: plugin->fx_knob0[3] = data; break;
        case MULTIFX_FX3_KNOB1:        plugin->fx_knob1[3] = data; break;
        case MULTIFX_FX3_KNOB2: plugin->fx_knob2[3] = data; break;
        case MULTIFX_FX3_COMBOBOX: plugin->fx_combobox[3] = data; break;

        case MULTIFX_FX4_KNOB0: plugin->fx_knob0[4] = data; break;
        case MULTIFX_FX4_KNOB1:        plugin->fx_knob1[4] = data; break;
        case MULTIFX_FX4_KNOB2: plugin->fx_knob2[4] = data; break;
        case MULTIFX_FX4_COMBOBOX: plugin->fx_combobox[4] = data; break;

        case MULTIFX_FX5_KNOB0: plugin->fx_knob0[5] = data; break;
        case MULTIFX_FX5_KNOB1:        plugin->fx_knob1[5] = data; break;
        case MULTIFX_FX5_KNOB2: plugin->fx_knob2[5] = data; break;
        case MULTIFX_FX5_COMBOBOX: plugin->fx_combobox[5] = data; break;

        case MULTIFX_FX6_KNOB0: plugin->fx_knob0[6] = data; break;
        case MULTIFX_FX6_KNOB1:        plugin->fx_knob1[6] = data; break;
        case MULTIFX_FX6_KNOB2: plugin->fx_knob2[6] = data; break;
        case MULTIFX_FX6_COMBOBOX: plugin->fx_combobox[6] = data; break;

        case MULTIFX_FX7_KNOB0: plugin->fx_knob0[7] = data; break;
        case MULTIFX_FX7_KNOB1:        plugin->fx_knob1[7] = data; break;
        case MULTIFX_FX7_KNOB2: plugin->fx_knob2[7] = data; break;
        case MULTIFX_FX7_COMBOBOX: plugin->fx_combobox[7] = data; break;
    }
}

PluginHandle g_multifx_instantiate(PluginDescriptor * descriptor,
        int s_rate, fp_get_audio_pool_item_from_host a_host_audio_pool_func,
        int a_plugin_uid, fp_queue_message a_queue_func)
{
    t_multifx *plugin_data;
    hpalloc((void**)&plugin_data, sizeof(t_multifx));

    plugin_data->descriptor = descriptor;
    plugin_data->fs = s_rate;
    plugin_data->plugin_uid = a_plugin_uid;
    plugin_data->queue_func = a_queue_func;

    plugin_data->mono_modules =
            v_multifx_mono_init(plugin_data->fs, plugin_data->plugin_uid);

    plugin_data->i_slow_index =
            MULTIFX_SLOW_INDEX_ITERATIONS + MULTIFX_AMORITIZER;

    ++MULTIFX_AMORITIZER;
    if(MULTIFX_AMORITIZER >= MULTIFX_SLOW_INDEX_ITERATIONS)
    {
        MULTIFX_AMORITIZER = 0;
    }

    plugin_data->is_on = 0;

    plugin_data->port_table = g_get_port_table(
        (void**)plugin_data, descriptor);

    v_cc_map_init(&plugin_data->cc_map);

    return (PluginHandle) plugin_data;
}

void v_multifx_load(PluginHandle instance,
        PluginDescriptor * Descriptor, char * a_file_path)
{
    t_multifx *plugin_data = (t_multifx*)instance;
    generic_file_loader(instance, Descriptor,
        a_file_path, plugin_data->port_table, &plugin_data->cc_map);
}

void v_multifx_set_port_value(PluginHandle Instance,
        int a_port, SGFLT a_value)
{
    t_multifx *plugin_data = (t_multifx*)Instance;
    plugin_data->port_table[a_port] = a_value;
}

void v_multifx_check_if_on(t_multifx *plugin_data)
{
    int f_i = 0;

    while(f_i < 8)
    {
        plugin_data->mono_modules->fx_func_ptr[f_i] =
            g_mf3_get_function_pointer((int)(*(plugin_data->fx_combobox[f_i])));

        if(plugin_data->mono_modules->fx_func_ptr[f_i] != v_mf3_run_off)
        {
            plugin_data->is_on = 1;
        }

        ++f_i;
    }
}

void v_multifx_process_midi_event(
    t_multifx * plugin_data, t_seq_event * a_event)
{
    if (a_event->type == EVENT_CONTROLLER)
    {
        assert(a_event->param >= 1 && a_event->param < 128);

        plugin_data->midi_event_types[plugin_data->midi_event_count] =
                EVENT_CONTROLLER;
        plugin_data->midi_event_ticks[plugin_data->midi_event_count] =
                a_event->tick;
        plugin_data->midi_event_ports[plugin_data->midi_event_count] =
                a_event->param;
        plugin_data->midi_event_values[plugin_data->midi_event_count] =
                a_event->value;

        if(!plugin_data->is_on)
        {
            v_multifx_check_if_on(plugin_data);

            //Meaning that we now have set the port anyways because the
            //main loop won't be running
            if(!plugin_data->is_on)
            {
                plugin_data->port_table[plugin_data->midi_event_ports[
                        plugin_data->midi_event_count]] =
                        plugin_data->midi_event_values[
                        plugin_data->midi_event_count];
            }
        }

        ++plugin_data->midi_event_count;
    }
}

void v_multifx_run(
        PluginHandle instance, int sample_count,
        struct ShdsList * midi_events, struct ShdsList *atm_events)
{
    t_multifx *plugin_data = (t_multifx*)instance;
    t_mf3_multi * f_fx;

    t_seq_event **events = (t_seq_event**)midi_events->data;
    int event_count = midi_events->len;

    int event_pos;
    int midi_event_pos = 0;
    plugin_data->midi_event_count = 0;

    for(event_pos = 0; event_pos < event_count; ++event_pos)
    {
        v_multifx_process_midi_event(plugin_data, events[event_pos]);
    }

    int f_i = 0;

    v_plugin_event_queue_reset(&plugin_data->atm_queue);

    t_seq_event * ev_tmp;
    for(f_i = 0; f_i < atm_events->len; ++f_i)
    {
        ev_tmp = (t_seq_event*)atm_events->data[f_i];
        v_plugin_event_queue_add(
            &plugin_data->atm_queue, ev_tmp->type,
            ev_tmp->tick, ev_tmp->value, ev_tmp->port);
    }

    f_i = 0;

    if(plugin_data->i_slow_index >= MULTIFX_SLOW_INDEX_ITERATIONS)
    {
        plugin_data->i_slow_index -= MULTIFX_SLOW_INDEX_ITERATIONS;
        plugin_data->is_on = 0;
        v_multifx_check_if_on(plugin_data);
    }
    else
    {
        ++plugin_data->i_slow_index;
    }

    f_i = 0;

    if(plugin_data->is_on)
    {
        int i_mono_out = 0;

        while((i_mono_out) < sample_count)
        {
            while(midi_event_pos < plugin_data->midi_event_count &&
                plugin_data->midi_event_ticks[midi_event_pos] == i_mono_out)
            {
                if(plugin_data->midi_event_types[midi_event_pos] ==
                        EVENT_CONTROLLER)
                {
                    v_cc_map_translate(
                        &plugin_data->cc_map, plugin_data->descriptor,
                        plugin_data->port_table,
                        plugin_data->midi_event_ports[midi_event_pos],
                        plugin_data->midi_event_values[midi_event_pos]);
                }
                ++midi_event_pos;
            }

            v_plugin_event_queue_atm_set(
                &plugin_data->atm_queue, i_mono_out,
                plugin_data->port_table);

            plugin_data->mono_modules->current_sample0 =
                    plugin_data->output0[(i_mono_out)];
            plugin_data->mono_modules->current_sample1 =
                    plugin_data->output1[(i_mono_out)];

            f_i = 0;

            while(f_i < 8)
            {
                if(plugin_data->mono_modules->fx_func_ptr[f_i] != v_mf3_run_off)
                {
                    f_fx = &plugin_data->mono_modules->multieffect[f_i];
                    v_sml_run(&plugin_data->mono_modules->smoothers[f_i][0],
                            *plugin_data->fx_knob0[f_i]);
                    v_sml_run(&plugin_data->mono_modules->smoothers[f_i][1],
                            *plugin_data->fx_knob1[f_i]);
                    v_sml_run(&plugin_data->mono_modules->smoothers[f_i][2],
                            *plugin_data->fx_knob2[f_i]);

                    v_mf3_set(f_fx,
                    plugin_data->mono_modules->smoothers[f_i][0].last_value,
                    plugin_data->mono_modules->smoothers[f_i][1].last_value,
                    plugin_data->mono_modules->smoothers[f_i][2].last_value);

                    plugin_data->mono_modules->fx_func_ptr[f_i](
                        f_fx,
                        (plugin_data->mono_modules->current_sample0),
                        (plugin_data->mono_modules->current_sample1));

                    plugin_data->mono_modules->current_sample0 = f_fx->output0;
                    plugin_data->mono_modules->current_sample1 = f_fx->output1;
                }
                ++f_i;
            }

            plugin_data->output0[(i_mono_out)] =
                    (plugin_data->mono_modules->current_sample0);
            plugin_data->output1[(i_mono_out)] =
                    (plugin_data->mono_modules->current_sample1);

            ++i_mono_out;
        }
    }
}

PluginDescriptor *multifx_plugin_descriptor(){
    PluginDescriptor *f_result = get_pyfx_descriptor(MULTIFX_COUNT);

    set_pyfx_port(f_result, MULTIFX_FX0_KNOB0, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX0_KNOB1, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX0_KNOB2, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX0_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    set_pyfx_port(f_result, MULTIFX_FX1_KNOB0, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX1_KNOB1, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX1_KNOB2, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX1_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    set_pyfx_port(f_result, MULTIFX_FX2_KNOB0, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX2_KNOB1, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX2_KNOB2, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX2_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    set_pyfx_port(f_result, MULTIFX_FX3_KNOB0, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX3_KNOB1, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX3_KNOB2, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX3_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    set_pyfx_port(f_result, MULTIFX_FX4_KNOB0, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX4_KNOB1, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX4_KNOB2, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX4_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    set_pyfx_port(f_result, MULTIFX_FX5_KNOB0, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX5_KNOB1, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX5_KNOB2, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX5_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    set_pyfx_port(f_result, MULTIFX_FX6_KNOB0, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX6_KNOB1, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX6_KNOB2, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX6_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);
    set_pyfx_port(f_result, MULTIFX_FX7_KNOB0, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX7_KNOB1, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX7_KNOB2, 64.0f, 0.0f, 127.0f);
    set_pyfx_port(f_result, MULTIFX_FX7_COMBOBOX, 0.0f, 0.0f, MULTIFX3KNOB_MAX_INDEX);


    f_result->cleanup = v_multifx_cleanup;
    f_result->connect_port = v_multifx_connect_port;
    f_result->connect_buffer = v_multifx_connect_buffer;
    f_result->instantiate = g_multifx_instantiate;
    f_result->panic = v_multifx_panic;
    f_result->load = v_multifx_load;
    f_result->set_port_value = v_multifx_set_port_value;
    f_result->set_cc_map = v_multifx_set_cc_map;

    f_result->API_Version = 1;
    f_result->configure = NULL;
    f_result->run_replacing = v_multifx_run;
    f_result->on_stop = v_multifx_on_stop;
    f_result->offline_render_prep = NULL;

    return f_result;
}


t_multifx_mono_modules * v_multifx_mono_init(SGFLT a_sr, int a_plugin_uid){
    t_multifx_mono_modules * a_mono;
    hpalloc((void**)&a_mono, sizeof(t_multifx_mono_modules));

    int f_i = 0;

    while(f_i < 8)
    {
        g_mf3_init(&a_mono->multieffect[f_i], a_sr, 1);
        a_mono->fx_func_ptr[f_i] = v_mf3_run_off;
        int f_i2 = 0;
        while(f_i2 < 3)
        {
            g_sml_init(&a_mono->smoothers[f_i][f_i2],
                a_sr, 127.0f, 0.0f, 0.1f);
            ++f_i2;
        }
        ++f_i;
    }

    g_sml_init(&a_mono->time_smoother, a_sr, 100.0f, 10.0f, 0.1f);

    a_mono->vol_linear = 1.0f;

    return a_mono;
}

/*
void v_multifx_destructor()
{
    if (f_result) {
        free((PluginPortDescriptor *) f_result->PortDescriptors);
        free((char **) f_result->PortNames);
        free((PluginPortRangeHint *) f_result->PortRangeHints);
        free(f_result);
    }
    if (LMSDDescriptor) {
        free(LMSDDescriptor);
    }
}
*/