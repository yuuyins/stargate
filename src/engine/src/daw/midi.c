#include <string.h>

#include "stargate.h"
#include "daw.h"
#include "files.h"


void v_daw_set_midi_devices(){
#ifndef NO_MIDI

    char f_path[2048];
    int f_i, f_i2;
    t_midi_device * f_device;

    if(!STARGATE->midi_devices)
    {
        return;
    }

    sprintf(f_path, "%s%sprojects%sdaw%smidi_routing.txt",
        STARGATE->project_folder, PATH_SEP, PATH_SEP, PATH_SEP);

    if(!i_file_exists(f_path))
    {
        return;
    }

    t_2d_char_array * f_current_string =
        g_get_2d_array_from_file(f_path, LARGE_STRING);

    for(f_i = 0; f_i < DN_TRACK_COUNT; ++f_i)
    {
        v_iterate_2d_char_array(f_current_string);
        if(f_current_string->eof)
        {
            break;
        }

        int f_on = atoi(f_current_string->current_str);

        v_iterate_2d_char_array(f_current_string);
        int f_track_num = atoi(f_current_string->current_str);

        v_iterate_2d_char_array_to_next_line(f_current_string);

        for(f_i2 = 0; f_i2 < STARGATE->midi_devices->count; ++f_i2)
        {
            f_device = &STARGATE->midi_devices->devices[f_i2];
            if(!strcmp(f_current_string->current_str, f_device->name))
            {
                v_daw_set_midi_device(f_on, f_i2, f_track_num);
                break;
            }
        }
    }

    g_free_2d_char_array(f_current_string);

#endif

}


#ifndef NO_MIDI

void v_daw_set_midi_device(int a_on, int a_device, int a_output)
{
    t_daw * self = DAW;
    /* Interim logic to get a minimum viable product working
     * TODO:  Make it modular and able to support multiple devices
     */
    t_daw_midi_routing_list * f_list = &self->midi_routing;
    t_midi_routing * f_route = &f_list->routes[a_device];
    t_pytrack * f_track_old = NULL;
    t_pytrack * f_track_new = self->track_pool[a_output];

    if(f_route->output_track != -1)
    {
        f_track_old = self->track_pool[f_route->output_track];
    }

    if(f_track_old && (!f_route->on || f_route->output_track != a_output))
    {
        f_track_old->extern_midi = 0;
        f_track_old->extern_midi_count = &ZERO;
        f_track_old->midi_device = 0;
    }

    f_route->on = a_on;
    f_route->output_track = a_output;

    if(f_route->on && STARGATE->midi_devices->devices[a_device].loaded)
    {
        f_track_new->midi_device = &STARGATE->midi_devices->devices[a_device];
        f_track_new->extern_midi =
            STARGATE->midi_devices->devices[a_device].instanceEventBuffers;

        midiPoll(f_track_new->midi_device);
        midiDeviceRead(f_track_new->midi_device,
            STARGATE->thread_storage[0].sample_rate, 512);

        STARGATE->midi_devices->devices[a_device].instanceEventCounts = 0;
        STARGATE->midi_devices->devices[a_device].midiEventReadIndex = 0;
        STARGATE->midi_devices->devices[a_device].midiEventWriteIndex = 0;

        f_track_new->extern_midi_count =
            &STARGATE->midi_devices->devices[a_device].instanceEventCounts;
    }
    else
    {
        f_track_new->extern_midi = 0;
        f_track_new->extern_midi_count = &ZERO;
        f_track_new->midi_device = 0;
    }
}

#endif

void v_daw_process_external_midi(
    t_daw * self,
    t_pytrack * a_track,
    int sample_count,
    int a_thread_num,
    t_daw_thread_storage * a_ts
){
    if(!a_track->midi_device)
    {
        return;
    }

    SGFLT f_sample_rate = STARGATE->thread_storage[a_thread_num].sample_rate;
    int f_playback_mode = STARGATE->playback_mode;
    int f_midi_learn = STARGATE->midi_learn;
    SGFLT f_tempo = self->ts[0].tempo;

    midiPoll(a_track->midi_device);
    midiDeviceRead(a_track->midi_device, f_sample_rate, sample_count);

    int f_extern_midi_count = *a_track->extern_midi_count;
    t_seq_event * events = a_track->extern_midi;

    assert(f_extern_midi_count < 200);

    int f_i2 = 0;

    int f_valid_type;

    char * f_osc_msg = a_track->osc_cursor_message;

    while(f_i2 < f_extern_midi_count)
    {
        f_valid_type = 1;

        if(events[f_i2].tick >= sample_count)
        {
            //Otherwise the event will be missed
            events[f_i2].tick = sample_count - 1;
        }

        if(events[f_i2].type == EVENT_NOTEON)
        {
            if(f_playback_mode == PLAYBACK_MODE_REC)
            {
                SGFLT f_beat = a_ts->ml_current_beat +
                    f_samples_to_beat_count(events[f_i2].tick,
                        f_tempo, f_sample_rate);

                sprintf(f_osc_msg, "on|%f|%i|%i|%i|%ld",
                    f_beat, a_track->track_num, events[f_i2].note,
                    events[f_i2].velocity,
                    a_ts->current_sample + events[f_i2].tick);
                v_queue_osc_message("mrec", f_osc_msg);
            }

            sprintf(f_osc_msg, "1|%i", events[f_i2].note);
            v_queue_osc_message("ne", f_osc_msg);

        }
        else if(events[f_i2].type == EVENT_NOTEOFF)
        {
            if(f_playback_mode == PLAYBACK_MODE_REC)
            {
                SGFLT f_beat = a_ts->ml_current_beat +
                    f_samples_to_beat_count(events[f_i2].tick,
                        f_tempo, f_sample_rate);

                sprintf(f_osc_msg, "off|%f|%i|%i|%ld",
                    f_beat, a_track->track_num, events[f_i2].note,
                    a_ts->current_sample + events[f_i2].tick);
                v_queue_osc_message("mrec", f_osc_msg);
            }

            sprintf(f_osc_msg, "0|%i", events[f_i2].note);
            v_queue_osc_message("ne", f_osc_msg);
        }
        else if(events[f_i2].type == EVENT_PITCHBEND)
        {
            if(f_playback_mode == PLAYBACK_MODE_REC)
            {
                SGFLT f_beat = a_ts->ml_current_beat +
                    f_samples_to_beat_count(events[f_i2].tick,
                        f_tempo, f_sample_rate);

                sprintf(f_osc_msg, "pb|%f|%i|%f|%ld",
                    f_beat, a_track->track_num, events[f_i2].value,
                    a_ts->current_sample + events[f_i2].tick);
                v_queue_osc_message("mrec", f_osc_msg);
            }
        }
        else if(events[f_i2].type == EVENT_CONTROLLER)
        {
            int controller = events[f_i2].param;

            if(f_midi_learn)
            {
                STARGATE->midi_learn = 0;
                f_midi_learn = 0;
                sprintf(f_osc_msg, "%i", controller);
                v_queue_osc_message("ml", f_osc_msg);
            }

            /*SGFLT f_start =
                ((self->playback_cursor) +
                ((((SGFLT)(events[f_i2].tick)) / ((SGFLT)sample_count))
                * (self->playback_inc))) * 4.0f;*/
            v_set_control_from_cc(&events[f_i2], a_track);

            if(f_playback_mode == PLAYBACK_MODE_REC)
            {
                SGFLT f_beat =
                    a_ts->ml_current_beat +
                    f_samples_to_beat_count(
                        events[f_i2].tick, f_tempo,
                        f_sample_rate);

                sprintf(f_osc_msg,
                    "cc|%f|%i|%i|%f|%ld",
                    f_beat,
                    a_track->track_num, controller, events[f_i2].value,
                    a_ts->current_sample + events[f_i2].tick);
                v_queue_osc_message("mrec", f_osc_msg);
            }
        }
        else
        {
            f_valid_type = 0;
        }

        if(f_valid_type)
        {
            shds_list_append(a_track->event_list, &events[f_i2]);
        }

        ++f_i2;
    }

    shds_list_isort(a_track->event_list, seq_event_cmpfunc);
}

void v_daw_process_note_offs(
    t_daw * self,
    int f_i,
    t_daw_thread_storage * a_ts
){
    t_pytrack * f_track = self->track_pool[f_i];
    long f_current_sample = a_ts->current_sample;
    long f_next_current_sample = a_ts->f_next_current_sample;

    int f_i2 = 0;
    long f_note_off;

    while(f_i2 < MIDI_NOTE_COUNT)
    {
        f_note_off = f_track->note_offs[f_i2];
        if(f_note_off >= f_current_sample &&
           f_note_off < f_next_current_sample)
        {
            t_seq_event * f_event =
                &f_track->event_buffer[f_track->period_event_index];
            v_ev_clear(f_event);

            v_ev_set_noteoff(f_event, 0, f_i2, 0);
            f_event->tick = f_note_off - f_current_sample;
            ++f_track->period_event_index;
            f_track->note_offs[f_i2] = -1;

            shds_list_append(f_track->event_list, f_event);
        }
        ++f_i2;
    }
}

void v_daw_process_midi(
    t_daw * self,
    t_daw_item_ref * a_item_ref,
    int a_track_num,
    int sample_count,
    int a_playback_mode,
    long a_current_sample,
    t_daw_thread_storage * a_ts
){
    t_daw_item * f_current_item;
    double f_adjusted_start;
    int f_i;
    t_pytrack * f_track = self->track_pool[a_track_num];
    f_track->period_event_index = 0;

    double f_track_current_period_beats = (a_ts->ml_current_beat);
    double f_track_next_period_beats = (a_ts->ml_next_beat);
    double f_track_beats_offset = 0.0f;

    if((!self->overdub_mode) && (a_playback_mode == 2) &&
        (f_track->extern_midi))
    {

    }
    else if(a_playback_mode > 0)
    {
        while(1)
        {
            f_current_item = self->item_pool[a_item_ref->item_uid];

            if((f_track->item_event_index) >= (f_current_item->event_count))
            {
                break;
            }

            t_seq_event * f_event =
                &f_current_item->events[f_track->item_event_index];

            f_adjusted_start = f_event->start + a_item_ref->start -
                a_item_ref->start_offset;

            if(f_adjusted_start < f_track_current_period_beats)
            {
                ++f_track->item_event_index;
                continue;
            }

            if((f_adjusted_start >= f_track_current_period_beats) &&
                (f_adjusted_start < f_track_next_period_beats) &&
                (f_adjusted_start < a_item_ref->end))
            {
                if(f_event->type == EVENT_NOTEON)
                {
                    int f_note_sample_offset = 0;
                    double f_note_start_diff =
                        f_adjusted_start - f_track_current_period_beats +
                        f_track_beats_offset;
                    double f_note_start_frac = f_note_start_diff /
                            (a_ts->ml_sample_period_inc_beats);
                    f_note_sample_offset =  (int)(f_note_start_frac *
                            ((SGFLT)sample_count));

                    if(f_track->note_offs[f_event->note] >= a_current_sample)
                    {
                        t_seq_event * f_buff_ev;

                        /*There's already a note_off scheduled ahead of
                         * this one, process it immediately to avoid
                         * hung notes*/
                        f_buff_ev = &f_track->event_buffer[
                            f_track->period_event_index];
                        v_ev_clear(f_buff_ev);

                        v_ev_set_noteoff(f_buff_ev, 0,
                                (f_event->note), 0);
                        f_buff_ev->tick = f_note_sample_offset;

                        ++f_track->period_event_index;
                    }

                    t_seq_event * f_buff_ev =
                        &f_track->event_buffer[f_track->period_event_index];

                    v_ev_clear(f_buff_ev);

                    v_ev_set_noteon(f_buff_ev, 0,
                            f_event->note, f_event->velocity);

                    f_buff_ev->tick = f_note_sample_offset;

                    ++f_track->period_event_index;

                    f_track->note_offs[(f_event->note)] =
                        a_current_sample + ((int)(f_event->length *
                        a_ts->samples_per_beat));
                }
                else if(f_event->type == EVENT_CONTROLLER)
                {
                    int controller = f_event->param;

                    t_seq_event * f_buff_ev =
                        &f_track->event_buffer[f_track->period_event_index];

                    int f_note_sample_offset = 0;

                    double f_note_start_diff =
                        ((f_adjusted_start) - f_track_current_period_beats) +
                        f_track_beats_offset;
                    double f_note_start_frac = f_note_start_diff /
                        a_ts->ml_sample_period_inc_beats;
                    f_note_sample_offset =
                        (int)(f_note_start_frac * ((SGFLT)sample_count));

                    v_ev_clear(f_buff_ev);

                    v_ev_set_controller(
                        f_buff_ev, 0, controller, f_event->value);

                    v_set_control_from_cc(f_buff_ev, f_track);

                    f_buff_ev->tick = f_note_sample_offset;

                    ++f_track->period_event_index;
                }
                else if(f_event->type == EVENT_PITCHBEND)
                {
                    int f_note_sample_offset = 0;
                    double f_note_start_diff = ((f_adjusted_start) -
                        f_track_current_period_beats) + f_track_beats_offset;
                    double f_note_start_frac = f_note_start_diff /
                        a_ts->ml_sample_period_inc_beats;
                    f_note_sample_offset =  (int)(f_note_start_frac *
                        ((SGFLT)sample_count));

                    t_seq_event * f_buff_ev =
                        &f_track->event_buffer[f_track->period_event_index];

                    v_ev_clear(f_buff_ev);
                    v_ev_set_pitchbend(f_buff_ev, 0, f_event->value);
                    f_buff_ev->tick = f_note_sample_offset;

                    ++f_track->period_event_index;
                }

                ++f_track->item_event_index;
            }
            else
            {
                break;
            }
        }
    }

    for(f_i = 0; f_i < f_track->period_event_index; ++f_i)
    {
        shds_list_append(f_track->event_list, &f_track->event_buffer[f_i]);
    }
}
