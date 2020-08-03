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

#include "gr_demod_ssb_sdr.h"

gr_demod_ssb_sdr_sptr make_gr_demod_ssb_sdr(int sps, int samp_rate, int carrier_freq,
                                          int filter_width, int sb)
{
    std::vector<int> signature;
    signature.push_back(sizeof (gr_complex));
    signature.push_back(sizeof (float));
    return gnuradio::get_initial_sptr(new gr_demod_ssb_sdr(signature, sps, samp_rate, carrier_freq,
                                                      filter_width, sb));
}



gr_demod_ssb_sdr::gr_demod_ssb_sdr(std::vector<int>signature, int sps, int samp_rate, int carrier_freq,
                                 int filter_width, int sb) :
    gr::hier_block2 ("gr_demod_ssb_sdr",
                      gr::io_signature::make (1, 1, sizeof (gr_complex)),
                      gr::io_signature::makev (2, 2, signature))
{
    _target_samp_rate = 8000;
    _samp_rate = samp_rate;
    _sps = sps;
    _carrier_freq = carrier_freq;
    _filter_width = filter_width;
    gr::calculate_deemph_taps(8000, 65e-6, _ataps, _btaps);

    std::vector<float> taps = gr::filter::firdes::low_pass(_sps, _samp_rate, _target_samp_rate/2,
                            _target_samp_rate/2, gr::filter::firdes::WIN_BLACKMAN_HARRIS);
    _resampler = gr::filter::rational_resampler_base_ccf::make(1,_sps,taps);

    _if_gain = gr::blocks::multiply_const_cc::make(0.9);

    _filter_usb = gr::filter::fft_filter_ccc::make(1, gr::filter::firdes::complex_band_pass_2(
            1, _target_samp_rate, 200, _filter_width,250, 60, gr::filter::firdes::WIN_BLACKMAN_HARRIS));
    _filter_lsb = gr::filter::fft_filter_ccc::make(1, gr::filter::firdes::complex_band_pass_2(
            1, _target_samp_rate, -_filter_width, -200,250, 60, gr::filter::firdes::WIN_BLACKMAN_HARRIS));
    _squelch = gr::analog::pwr_squelch_cc::make(-140,0.01,0,true);
    _feed_forward_agc = gr::analog::feedforward_agc_cc::make(320,1);
    _agc = gr::analog::agc2_cc::make(1e-2, 1e-4, 0.8, 1);
    _agc->set_max_gain(0.9);
    _rail = gr::analog::rail_ff::make(-1.0, 1.0);
    _audio_filter = gr::filter::fft_filter_fff::make(
                1,gr::filter::firdes::band_pass_2(
                    2, _target_samp_rate, 300, _filter_width, 200, 60, gr::filter::firdes::WIN_BLACKMAN_HARRIS));
    _complex_to_real = gr::blocks::complex_to_real::make();
    _de_emph_filter = gr::filter::iir_filter_ffd::make(_ataps, _btaps, false);
    _level_control = gr::blocks::multiply_const_ff::make(0.99);


    connect(self(),0,_resampler,0);
    connect(_resampler,0,_if_gain,0);
    if(!sb)
    {
        connect(_if_gain,0,_filter_usb,0);
        connect(_filter_usb,0,self(),0);
        connect(_filter_usb,0,_squelch,0);
    }
    else
    {
        connect(_if_gain,0,_filter_lsb,0);
        connect(_filter_lsb,0,self(),0);
        connect(_filter_lsb,0,_squelch,0);
    }
    connect(_squelch,0,_agc,0);
    connect(_agc,0,_complex_to_real,0);
    connect(_complex_to_real,0,_rail,0);
    connect(_rail,0,_de_emph_filter,0);
    connect(_de_emph_filter,0,_level_control,0);
    connect(_level_control,0,_audio_filter,0);
    connect(_audio_filter,0,self(),1);

}


void gr_demod_ssb_sdr::set_filter_width(int filter_width)
{
    _filter_width = filter_width;
    std::vector<gr_complex> filter_usb_taps = gr::filter::firdes::complex_band_pass_2(
                1, _target_samp_rate, 300, _filter_width,250, 60, gr::filter::firdes::WIN_BLACKMAN_HARRIS);
    std::vector<gr_complex> filter_lsb_taps = gr::filter::firdes::complex_band_pass_2(
                1, _target_samp_rate, -_filter_width, -300,250, 60, gr::filter::firdes::WIN_BLACKMAN_HARRIS);

    _filter_usb->set_taps(filter_usb_taps);
    _filter_lsb->set_taps(filter_lsb_taps);
    _audio_filter->set_taps(gr::filter::firdes::band_pass_2(
            2, _target_samp_rate, 300, _filter_width, 250, 40, gr::filter::firdes::WIN_BLACKMAN_HARRIS));
}

void gr_demod_ssb_sdr::set_squelch(int value)
{
    _squelch->set_threshold(value);
}

void gr_demod_ssb_sdr::set_agc_attack(float value)
{
    _agc->set_attack_rate(value);
}

void gr_demod_ssb_sdr::set_agc_decay(float value)
{
    _agc->set_decay_rate(value);
}

void gr_demod_ssb_sdr::set_gain(float value)
{
    _if_gain->set_k(value);
}
