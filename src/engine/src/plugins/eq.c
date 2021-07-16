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
#include "audiodsp/lib/spectrum_analyzer.h"
#include "plugins/eq.h"


void v_sgeq_cleanup(PluginHandle instance)
{
    free(instance);
}

void v_sgeq_set_cc_map(PluginHandle instance, char * a_msg)
{
    t_sgeq *plugin = (t_sgeq *)instance;
    v_generic_cc_map_set(&plugin->cc_map, a_msg);
}

void v_sgeq_panic(PluginHandle instance)
{
    //t_sgeq *plugin = (t_sgeq*)instance;
}

void v_sgeq_on_stop(PluginHandle instance)
{
    //t_sgeq *plugin = (t_sgeq*)instance;
}

void v_sgeq_connect_buffer(PluginHandle instance, int a_index,
        SGFLT * DataLocation, int a_is_sidechain)
{
    if(a_is_sidechain)
    {
        return;
    }

    t_sgeq *plugin = (t_sgeq*)instance;

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

void v_sgeq_connect_port(PluginHandle instance, int port,
        PluginData * data)
{
    t_sgeq *plugin;

    plugin = (t_sgeq *) instance;

    switch (port)
    {
        case SGEQ_EQ1_FREQ: plugin->eq_freq[0] = data; break;
        case SGEQ_EQ2_FREQ: plugin->eq_freq[1] = data; break;
        case SGEQ_EQ3_FREQ: plugin->eq_freq[2] = data; break;
        case SGEQ_EQ4_FREQ: plugin->eq_freq[3] = data; break;
        case SGEQ_EQ5_FREQ: plugin->eq_freq[4] = data; break;
        case SGEQ_EQ6_FREQ: plugin->eq_freq[5] = data; break;

        case SGEQ_EQ1_RES: plugin->eq_res[0] = data; break;
        case SGEQ_EQ2_RES: plugin->eq_res[1] = data; break;
        case SGEQ_EQ3_RES: plugin->eq_res[2] = data; break;
        case SGEQ_EQ4_RES: plugin->eq_res[3] = data; break;
        case SGEQ_EQ5_RES: plugin->eq_res[4] = data; break;
        case SGEQ_EQ6_RES: plugin->eq_res[5] = data; break;

        case SGEQ_EQ1_GAIN: plugin->eq_gain[0] = data; break;
        case SGEQ_EQ2_GAIN: plugin->eq_gain[1] = data; break;
        case SGEQ_EQ3_GAIN: plugin->eq_gain[2] = data; break;
        case SGEQ_EQ4_GAIN: plugin->eq_gain[3] = data; break;
        case SGEQ_EQ5_GAIN: plugin->eq_gain[4] = data; break;
        case SGEQ_EQ6_GAIN: plugin->eq_gain[5] = data; break;
        case SGEQ_SPECTRUM_ENABLED: plugin->spectrum_analyzer_on = data; break;
    }
}

PluginHandle g_sgeq_instantiate(PluginDescriptor * descriptor,
        int s_rate, fp_get_audio_pool_item_from_host a_host_audio_pool_func,
        int a_plugin_uid, fp_queue_message a_queue_func)
{
    t_sgeq *plugin_data;
    hpalloc((void**)&plugin_data, sizeof(t_sgeq));

    plugin_data->descriptor = descriptor;
    plugin_data->fs = s_rate;
    plugin_data->plugin_uid = a_plugin_uid;
    plugin_data->queue_func = a_queue_func;

    plugin_data->mono_modules =
            v_sgeq_mono_init(plugin_data->fs, plugin_data->plugin_uid);

    plugin_data->port_table = g_get_port_table(
        (void**)plugin_data, descriptor);

    v_cc_map_init(&plugin_data->cc_map);

    return (PluginHandle) plugin_data;
}

void v_sgeq_load(PluginHandle instance,
        PluginDescriptor * Descriptor, char * a_file_path)
{
    t_sgeq *plugin_data = (t_sgeq*)instance;
    generic_file_loader(instance, Descriptor,
        a_file_path, plugin_data->port_table, &plugin_data->cc_map);
}

void v_sgeq_set_port_value(PluginHandle Instance,
        int a_port, SGFLT a_value)
{
    t_sgeq *plugin_data = (t_sgeq*)Instance;
    plugin_data->port_table[a_port] = a_value;
}


void v_sgeq_process_midi_event(
    t_sgeq * plugin_data, t_seq_event * a_event)
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
        ++plugin_data->midi_event_count;
    }
}

void v_sgeq_run(
        PluginHandle instance, int sample_count,
        struct ShdsList * midi_events, struct ShdsList * atm_events)
{
    t_sgeq *plugin_data = (t_sgeq*)instance;

    t_seq_event **events = (t_seq_event**)midi_events->data;
    int event_count = midi_events->len;

    int f_i = 0;
    int midi_event_pos = 0;
    plugin_data->midi_event_count = 0;

    for(f_i = 0; f_i < event_count; ++f_i)
    {
        v_sgeq_process_midi_event(plugin_data, events[f_i]);
    }

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

    while(f_i < sample_count)
    {
        while(midi_event_pos < plugin_data->midi_event_count &&
            plugin_data->midi_event_ticks[midi_event_pos] == f_i)
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
            &plugin_data->atm_queue, f_i,
            plugin_data->port_table);

        int f_i2 = 0;

        while(f_i2 < SGEQ_EQ_COUNT)
        {
            if(*plugin_data->eq_gain[f_i2] != 0.0f)
            {
                v_pkq_calc_coeffs(&plugin_data->mono_modules->eqs[f_i2],
                        *plugin_data->eq_freq[f_i2],
                        *plugin_data->eq_res[f_i2] * 0.01f,
                        *plugin_data->eq_gain[f_i2] * 0.1f);
                v_pkq_run(&plugin_data->mono_modules->eqs[f_i2],
                        plugin_data->output0[f_i],
                        plugin_data->output1[f_i]);
                plugin_data->output0[f_i] =
                        plugin_data->mono_modules->eqs[f_i2].output0;
                plugin_data->output1[f_i] =
                        plugin_data->mono_modules->eqs[f_i2].output1;
            }
            ++f_i2;
        }

        ++f_i;
    }

    if((int)(*plugin_data->spectrum_analyzer_on))
    {
        v_spa_run(plugin_data->mono_modules->spectrum_analyzer,
                plugin_data->output0, plugin_data->output1, sample_count);
        if(plugin_data->mono_modules->spectrum_analyzer->str_buf[0] != '\0')
        {
            plugin_data->queue_func("ui",
                plugin_data->mono_modules->spectrum_analyzer->str_buf);
            plugin_data->mono_modules->spectrum_analyzer->str_buf[0] = '\0';
        }
    }

}

PluginDescriptor *sgeq_plugin_descriptor(){
    PluginDescriptor *f_result = get_pyfx_descriptor(SGEQ_COUNT);


    set_pyfx_port(f_result, SGEQ_EQ1_FREQ, 24.0f, 4.0f, 123.0f);
    set_pyfx_port(f_result, SGEQ_EQ2_FREQ, 42.0f, 4.0f, 123.0f);
    set_pyfx_port(f_result, SGEQ_EQ3_FREQ, 60.0f, 4.0f, 123.0f);
    set_pyfx_port(f_result, SGEQ_EQ4_FREQ, 78.0f, 4.0f, 123.0f);
    set_pyfx_port(f_result, SGEQ_EQ5_FREQ, 96.0f, 4.0f, 123.0f);
    set_pyfx_port(f_result, SGEQ_EQ6_FREQ, 114.0f, 4.0f, 123.0f);
    set_pyfx_port(f_result, SGEQ_EQ1_RES, 300.0f, 100.0f, 600.0f);
    set_pyfx_port(f_result, SGEQ_EQ2_RES, 300.0f, 100.0f, 600.0f);
    set_pyfx_port(f_result, SGEQ_EQ3_RES, 300.0f, 100.0f, 600.0f);
    set_pyfx_port(f_result, SGEQ_EQ4_RES, 300.0f, 100.0f, 600.0f);
    set_pyfx_port(f_result, SGEQ_EQ5_RES, 300.0f, 100.0f, 600.0f);
    set_pyfx_port(f_result, SGEQ_EQ6_RES, 300.0f, 100.0f, 600.0f);
    set_pyfx_port(f_result, SGEQ_EQ1_GAIN, 0.0f, -240.0f, 240.0f);
    set_pyfx_port(f_result, SGEQ_EQ2_GAIN, 0.0f, -240.0f, 240.0f);
    set_pyfx_port(f_result, SGEQ_EQ3_GAIN, 0.0f, -240.0f, 240.0f);
    set_pyfx_port(f_result, SGEQ_EQ4_GAIN, 0.0f, -240.0f, 240.0f);
    set_pyfx_port(f_result, SGEQ_EQ5_GAIN, 0.0f, -240.0f, 240.0f);
    set_pyfx_port(f_result, SGEQ_EQ6_GAIN, 0.0f, -240.0f, 240.0f);
    set_pyfx_port(f_result, SGEQ_SPECTRUM_ENABLED, 0.0f, 0.0f, 1.0f);

    f_result->cleanup = v_sgeq_cleanup;
    f_result->connect_port = v_sgeq_connect_port;
    f_result->connect_buffer = v_sgeq_connect_buffer;
    f_result->instantiate = g_sgeq_instantiate;
    f_result->panic = v_sgeq_panic;
    f_result->load = v_sgeq_load;
    f_result->set_port_value = v_sgeq_set_port_value;
    f_result->set_cc_map = v_sgeq_set_cc_map;

    f_result->API_Version = 1;
    f_result->configure = NULL;
    f_result->run_replacing = v_sgeq_run;
    f_result->on_stop = v_sgeq_on_stop;
    f_result->offline_render_prep = NULL;

    return f_result;
}

t_sgeq_mono_modules * v_sgeq_mono_init(SGFLT a_sr, int a_plugin_uid){
    t_sgeq_mono_modules * a_mono;
    hpalloc((void**)&a_mono, sizeof(t_sgeq_mono_modules));

    int f_i = 0;

    while(f_i < SGEQ_EQ_COUNT)
    {
        g_pkq_init(&a_mono->eqs[f_i], a_sr);
        ++f_i;
    }

    a_mono->vol_linear = 1.0f;

    a_mono->spectrum_analyzer =
        g_spa_spectrum_analyzer_get(4096, a_plugin_uid);

    return a_mono;
}

/*
void v_sgeq_destructor()
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