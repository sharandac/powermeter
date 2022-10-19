/**
 * @file measure_config.h
 * @author Dirk Broßwick (dirk.brosswick@googlemail.com)
 * @brief 
 * @version 1.0
 * @date 2022-10-03
 * 
 * @copyright Copyright (c) 2022
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */ 
#ifndef _MEASURE_CONFIG_H
    #define _MEASURE_CONFIG_H

    #include "utils/basejsonconfig.h"
    
    #define     MEASURE_JSON_CONFIG_FILE     "/measure.json"    /** @brief defines json config file name */
    /**
     * @brief 
     */

    /**
     * @brief ioport config structure
     */
    class measure_config_t : public BaseJsonConfig {
        public:
            measure_config_t();
            float network_frequency = 50;
            int samplerate_corr = 0;
            bool true_rms = false;
            
        protected:
            ////////////// Available for overloading: //////////////
            virtual bool onLoad(JsonDocument& document);
            virtual bool onSave(JsonDocument& document);
            virtual bool onDefault( void );
            virtual size_t getJsonBufferSize() { return 8192; }
    };
#endif // _MEASURE_CONFIG_H
