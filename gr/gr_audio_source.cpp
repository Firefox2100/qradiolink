// Written by Adrian Musceac YO8RZZ , started March 2016.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

#include "gr_audio_source.h"


gr_audio_source_sptr
make_gr_audio_source ()
{
    return gnuradio::get_initial_sptr(new gr_audio_source);
}

gr_audio_source::gr_audio_source() :
        gr::sync_block("gr_audio_source",
                       gr::io_signature::make (0, 0, 0),
                       gr::io_signature::make (1, 1, sizeof (float)))
{
    _offset = 0;
    _finished = true;
    _data = new std::vector<float>;
    /// Audio samples come in packets of 40 msec;
    set_output_multiple(320);
}

gr_audio_source::~gr_audio_source()
{
    delete _data;
}

void gr_audio_source::flush()
{
    gr::thread::scoped_lock guard(_mutex);
    _data->clear();
}

int gr_audio_source::set_data(std::vector<float> *data)
{

    gr::thread::scoped_lock guard(_mutex);
    _data->insert(_data->end(),data->begin(),data->end());
    delete data;
    _finished = false;
    return 0;
}

int gr_audio_source::work(int noutput_items,
       gr_vector_const_void_star &input_items,
       gr_vector_void_star &output_items)
{
    (void) input_items;
    gr::thread::scoped_lock guard(_mutex);
    if(_finished)
    {
        guard.unlock();
        struct timespec time_to_sleep = {0, 5000000L };
        nanosleep(&time_to_sleep, NULL);
        _finished = false;
        return 0;
    }


    float *out = (float*)(output_items[0]);
    unsigned n = std::min((unsigned)_data->size() - _offset,
                                  (unsigned)noutput_items);
    for(unsigned i=0;i < n;i++)
    {
        out[i] = _data->at(_offset + i);
    }

    _offset += n;
    if(_offset >= _data->size())
    {
        _data->clear();
        _finished = true;
        _offset = 0;
    }
    return n;
}
