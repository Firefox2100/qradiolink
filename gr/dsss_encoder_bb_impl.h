/* -*- c++ -*- */
/* 
 * Copyright 2014 Eric de Groot (edegroot@email.arizona.edu).
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_DSSS_DSSS_ENCODER_BB_IMPL_H
#define INCLUDED_DSSS_DSSS_ENCODER_BB_IMPL_H

#include <gnuradio/block.h>

namespace gr {
  namespace dsss {

      class dsss_encoder_bb : virtual public gr::block
      {
       public:
        typedef boost::shared_ptr<dsss_encoder_bb> sptr;

        /*!
         * \brief Return a shared_ptr to a new instance of dsss::dsss_encoder_bb.
         *
         * To avoid accidental use of raw pointers, dsss::dsss_encoder_bb'so
         * constructor is in a private implementation
         * class. dsss::dsss_encoder_bb::make is the public interface for
         * creating new instances.
         */
        static sptr make(const std::vector<int> &code);

        virtual std::vector<int> code() const = 0;
        virtual void set_code(const std::vector<int> &code) = 0;
      };

    class dsss_encoder_bb_impl : public dsss_encoder_bb
    {
     private:
      std::vector<int> d_code;

     public:
      dsss_encoder_bb_impl(const std::vector<int> &code);
      ~dsss_encoder_bb_impl();

      std::vector<int> code() const { return d_code; }
      void set_code(const std::vector<int> &code);

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
		       gr_vector_int &ninput_items,
		       gr_vector_const_void_star &input_items,
		       gr_vector_void_star &output_items);
    };

  } // namespace dsss
} // namespace gr

#endif /* INCLUDED_DSSS_DSSS_ENCODER_BB_IMPL_H */

