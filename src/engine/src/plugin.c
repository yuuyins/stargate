#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include "audiodsp/lib/lmalloc.h"
#include "plugin.h"
#include "plugins/sampler1.h"
#include "plugins/channel.h"
#include "plugins/compressor.h"
#include "plugins/delay.h"
#include "plugins/eq.h"
#include "plugins/limiter.h"
#include "plugins/limiter.h"
#include "plugins/vocoder.h"
#include "plugins/vocoder.h"
#include "plugins/multifx.h"
#include "plugins/va1.h"
#include "plugins/sidechain_comp.h"
#include "plugins/simple_fader.h"
#include "plugins/simple_reverb.h"
#include "plugins/trigger_fx.h"
#include "plugins/fm1.h"
#include "plugins/xfade.h"
#include "csv/2d.h"
#include "files.h"

PluginDescriptor_Function PLUGIN_DESC_FUNCS[] = {
    NULL, //0
    sampler1_plugin_descriptor, //1
    va1_plugin_descriptor, //2
    fm1_plugin_descriptor, //3
    multifx_plugin_descriptor, //4
    sgdelay_plugin_descriptor, //5
    sgeq_plugin_descriptor, //6
    sfader_plugin_descriptor, //7
    sreverb_plugin_descriptor, //8
    triggerfx_plugin_descriptor, //9
    scc_plugin_descriptor, //10
    sgchnl_plugin_descriptor, //11
    xfade_plugin_descriptor, //12
    sg_comp_plugin_descriptor, //13
    sg_vocoder_plugin_descriptor, //14
    sg_lim_plugin_descriptor //15
};


void v_cc_mapping_init(t_cc_mapping* self)
{
    int f_i;
    self->count = 0;

    for(f_i = 0; f_i < 5; ++f_i)
    {
        self->lows[f_i] = 0.0f;
        self->highs[f_i] = 1.0f;
        self->ports[f_i] = -1;
    }
}

void v_cc_mapping_set(t_cc_mapping* self, int a_port, SGFLT a_low, SGFLT a_high)
{
    self->ports[self->count] = a_port;
    self->lows[self->count] = a_low;
    self->highs[self->count] = a_high;
    self->count++;
}

void v_cc_map_init(t_plugin_cc_map * self)
{
    int f_i = 0;
    while(f_i < 128)
    {
        v_cc_mapping_init(&self->map[f_i]);
        f_i++;
    }
}

void v_cc_map_translate(t_plugin_cc_map *self, PluginDescriptor *desc,
    SGFLT *a_port_table, int a_cc, SGFLT a_value)
{
    int f_i;
    a_value *= 0.007874f;  // a_val / 127.0f

    for(f_i = 0; f_i < self->map[a_cc].count; ++f_i)
    {
        int f_port = self->map[a_cc].ports[f_i];
        PluginPortRangeHint * f_range = &desc->PortRangeHints[f_port];
        SGFLT f_diff = f_range->UpperBound - f_range->LowerBound;
        SGFLT f_min = f_diff * self->map[a_cc].lows[f_i];
        SGFLT f_max = f_diff * self->map[a_cc].highs[f_i];
        a_port_table[f_port] = (a_value * (f_max - f_min)) +
            f_min + f_range->LowerBound;
    }
}

SGFLT f_atm_to_ctrl_val(
    PluginDescriptor *self,
    int a_port,
    SGFLT a_val
){
    PluginPortRangeHint * f_range = &self->PortRangeHints[a_port];
    a_val *= 0.007874f;  // a_val / 127.0f
    return (a_val * (f_range->UpperBound - f_range->LowerBound)) +
        f_range->LowerBound;
}

void v_plugin_event_queue_add(
    t_plugin_event_queue *self, int a_type, int a_tick,
    SGFLT a_val, int a_port)
{
    t_plugin_event_queue_item * f_item = &self->items[self->count];
    f_item->type = a_type;
    f_item->tick = a_tick;
    f_item->value = a_val;
    f_item->port = a_port;
    ++self->count;
    assert(self->count <= 200);
}

void v_plugin_event_queue_reset(t_plugin_event_queue * self)
{
    self->pos = 0;
    self->count = 0;
}

t_plugin_event_queue_item * v_plugin_event_queue_iter(
    t_plugin_event_queue *self, int a_sample_num)
{
    t_plugin_event_queue_item * f_item = &self->items[self->pos];
    if(self->pos < self->count &&
       a_sample_num == f_item->tick)
    {
       ++self->pos;
       return f_item;
    }
    else
    {
        return 0;
    }
}

void v_plugin_event_queue_atm_set(
    t_plugin_event_queue *self,
    int a_sample_num,
    SGFLT * a_table
){
    while(1)
    {
        t_plugin_event_queue_item * f_item =
            v_plugin_event_queue_iter(self, a_sample_num);
        if(!f_item)
        {
            break;
        }

        a_table[f_item->port] = f_item->value;
    }
}

void v_ev_clear(t_seq_event * a_event)
{
    a_event->type = -1;
    a_event->tick = 0;
}

void v_ev_set_pitchbend(
    t_seq_event* a_event,
    int a_channel,
    int a_value
){
    a_event->type = EVENT_PITCHBEND;
    a_event->channel = a_channel;
    a_event->value = a_value;
}

void v_ev_set_noteoff(
    t_seq_event* a_event,
    int a_channel,
    int a_note,
    int a_velocity
){
    a_event->type = EVENT_NOTEOFF;
    a_event->channel = a_channel;
    a_event->note = a_note;
    a_event->velocity = a_velocity;
}

void v_ev_set_noteon(
    t_seq_event* a_event,
    int a_channel,
    int a_note,
    int a_velocity
){
    a_event->type = EVENT_NOTEON;
    a_event->channel = a_channel;
    a_event->note = a_note;
    a_event->velocity = a_velocity;
}

void v_ev_set_controller(
    t_seq_event* a_event,
    int a_channel,
    int a_cc_num,
    int a_value
){
    a_event->type = EVENT_CONTROLLER;
    a_event->channel = a_channel;
    a_event->param = a_cc_num;
    a_event->value = a_value;
}

void v_ev_set_atm(
    t_seq_event* a_event,
    int a_port_num,
    int a_value
){
    a_event->type = EVENT_AUTOMATION;
    a_event->channel = 0;
    a_event->port = a_port_num;
    a_event->value = a_value;
}

PluginDescriptor * get_pyfx_descriptor(int a_port_count){
    PluginDescriptor *f_result =
            (PluginDescriptor*)malloc(sizeof(PluginDescriptor));

    f_result->PortCount = a_port_count;

    f_result->PortDescriptors =
        (PluginPortDescriptor*)calloc(f_result->PortCount,
            sizeof(PluginPortDescriptor));

    f_result->PortRangeHints =
        (PluginPortRangeHint*)calloc(f_result->PortCount,
            sizeof(PluginPortRangeHint));

    return f_result;
}

void set_pyfx_port(
    PluginDescriptor * a_desc,
    int a_port,
    SGFLT a_default,
    SGFLT a_min,
    SGFLT a_max
){
    assert(a_port >= 0 && a_port < a_desc->PortCount);
    assert(!a_desc->PortDescriptors[a_port]);
    assert(a_min < a_max);
    assert(a_default >= a_min && a_default <= a_max);

    a_desc->PortDescriptors[a_port] = 1;
    a_desc->PortRangeHints[a_port].DefaultValue = a_default;
    a_desc->PortRangeHints[a_port].LowerBound = a_min;
    a_desc->PortRangeHints[a_port].UpperBound = a_max;
}



PluginData g_get_port_default(PluginDescriptor *plugin, int port)
{
    PluginPortRangeHint hint = plugin->PortRangeHints[port];
    assert(hint.DefaultValue <= hint.UpperBound &&
            hint.DefaultValue >= hint.LowerBound );
    return hint.DefaultValue;
}

SGFLT * g_get_port_table(
    PluginHandle * handle,
    PluginDescriptor * descriptor
){
    SGFLT * pluginControlIns;
    int j;

    int f_i = 0;

    hpalloc((void**)(&pluginControlIns), sizeof(SGFLT) * descriptor->PortCount);

    f_i = 0;
    while(f_i < descriptor->PortCount)
    {
        pluginControlIns[f_i] = 0.0f;
        f_i++;
    }

    for (j = 0; j < descriptor->PortCount; j++)
    {
        PluginPortDescriptor pod = descriptor->PortDescriptors[j];

        if(pod)
        {
            pluginControlIns[j] = g_get_port_default(descriptor, j);

            descriptor->connect_port(handle, j, &pluginControlIns[j]);
        }
    }

    return pluginControlIns;
}

void v_generic_cc_map_set(t_plugin_cc_map * a_cc_map, char * a_str)
{
    t_2d_char_array * f_2d_array = g_get_2d_array(SMALL_STRING);
    f_2d_array->array = a_str;
    v_iterate_2d_char_array(f_2d_array);
    int f_cc = atoi(f_2d_array->current_str);

    v_iterate_2d_char_array(f_2d_array);
    int f_count = atoi(f_2d_array->current_str);
    a_cc_map->map[f_cc].count = f_count;

    int f_i = 0;
    while(f_i < f_count)
    {
        v_iterate_2d_char_array(f_2d_array);
        int f_port = atoi(f_2d_array->current_str);
        v_iterate_2d_char_array(f_2d_array);
        SGFLT f_low = atof(f_2d_array->current_str);
        v_iterate_2d_char_array(f_2d_array);
        SGFLT f_high = atof(f_2d_array->current_str);

        v_cc_mapping_set(&a_cc_map->map[f_cc], f_port, f_low, f_high);

        ++f_i;
    }
}

void generic_file_loader(
    PluginHandle Instance,
    PluginDescriptor * Descriptor,
    char * a_path,
    SGFLT * a_table,
    t_plugin_cc_map * a_cc_map
){
    t_2d_char_array * f_2d_array = g_get_2d_array_from_file(a_path,
                LARGE_STRING);

    while(1)
    {
        v_iterate_2d_char_array(f_2d_array);

        if(f_2d_array->eof)
        {
            break;
        }

        assert(strcmp(f_2d_array->current_str, ""));

        if(f_2d_array->current_str[0] == 'c')
        {
            char * f_config_key = (char*)malloc(
                sizeof(char) * TINY_STRING);
            v_iterate_2d_char_array(f_2d_array);
            strcpy(f_config_key, f_2d_array->current_str);
            char * f_value = (char*)malloc(
                sizeof(char) * SMALL_STRING);
            v_iterate_2d_char_array_to_next_line(f_2d_array);
            strcpy(f_value, f_2d_array->current_str);

            Descriptor->configure(Instance, f_config_key, f_value, 0);

            free(f_config_key);
            free(f_value);
        }
        else if(f_2d_array->current_str[0] == 'm')
        {
            v_iterate_2d_char_array(f_2d_array);
            int f_cc = atoi(f_2d_array->current_str);

            v_iterate_2d_char_array(f_2d_array);
            int f_count = atoi(f_2d_array->current_str);

            int f_i = 0;
            while(f_i < f_count)
            {
                v_iterate_2d_char_array(f_2d_array);
                int f_port = atoi(f_2d_array->current_str);
                v_iterate_2d_char_array(f_2d_array);
                SGFLT f_low = atof(f_2d_array->current_str);
                v_iterate_2d_char_array(f_2d_array);
                SGFLT f_high = atof(f_2d_array->current_str);

                v_cc_mapping_set(&a_cc_map->map[f_cc], f_port, f_low, f_high);
                ++f_i;
            }
        }
        else
        {
            int f_port_key = atoi(f_2d_array->current_str);
            v_iterate_2d_char_array_to_next_line(f_2d_array);
            SGFLT f_port_value = atof(f_2d_array->current_str);

            assert(f_port_key >= 0);
            assert(f_port_key <= Descriptor->PortCount);

            a_table[f_port_key] = f_port_value;
        }
    }

    g_free_2d_char_array(f_2d_array);

}

NO_OPTIMIZATION void g_plugin_init(
    t_plugin * f_result,
    int a_sample_rate,
    int a_index,
    fp_get_audio_pool_item_from_host a_host_audio_pool_func,
    int a_plugin_uid,
    fp_queue_message a_queue_func
){
    f_result->active = 1;
    f_result->uid = a_index;
    f_result->pool_uid = a_plugin_uid;
    f_result->atm_count = 0;

    int buff_max = 512;

    f_result->atm_list = shds_list_new(buff_max, NULL);

    hpalloc(
        (void**)&f_result->atm_buffer,
        sizeof(t_seq_event) * buff_max
    );

    f_result->descfn = PLUGIN_DESC_FUNCS[a_index];

    f_result->descriptor = (PluginDescriptor*)f_result->descfn();

    assert(f_result->descriptor);
    printf("Calling descriptor->instantiate()\n");
    f_result->plugin_handle = (PluginHandle)f_result->descriptor->instantiate(
        f_result->descriptor,
        a_sample_rate,
        a_host_audio_pool_func,
        a_plugin_uid,
        a_queue_func
    );
    printf("Finished calling descriptor->instantiate()\n");

    f_result->solo = 0;
    f_result->mute = 0;
    f_result->power = 1;
}

int seq_event_cmpfunc(void *self, void *other){
    t_seq_event *self_ev = (t_seq_event*)self;
    t_seq_event *other_ev = (t_seq_event*)other;

    return self_ev->tick < other_ev->tick;
}
/*
void v_free_plugin(t_plugin * a_plugin)
{
    if(a_plugin)
    {
        if (a_plugin->descriptor->cleanup)
        {
            a_plugin->descriptor->cleanup(a_plugin->plugin_handle);
        }

        free(a_plugin);
    }
    else
    {
        printf("Error, attempted to free NULL plugin "
                "with v_free_plugin()\n");
    }
}
*/
