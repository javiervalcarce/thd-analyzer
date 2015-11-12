// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef THDANALYZER_WAVEFORM_GENERATOR_H_
#define THDANALYZER_WAVEFORM_GENERATOR_H_

#include <stdint.h>
#include <string>

namespace thd_analyzer {

      /**
       * Análisis de un tono puro. Estima la frecuencia central, su potencia y la de sus armónicos y con todo ello
       * calcula la distorsión harmónica total (THD - Total Harmonic Distortion).
       *
       */
      class WaveformGenerator {
      public:

            /**
             * Constructor.
             *
             * @param capture_device.
             */
            WaveformGenerator(const char* playback_pcm_device);

            /**
             * Destructor.
             */
            ~WaveformGenerator();

            /**
             * Inicialización.
             */
            int Init();

            /**
             */
            int Start();

            /**
             */
            int Stop();

            /**
             * Frecuencia cuyo coeficiente en la transformada tiene módulo máximo, expresado en Hz.
             */
            void SetFrequency(double frequency);

            /**
             * Amplitud máxima del pedazo de señal, normalizada entre 0.0 y 1.0, para convertir esto en voltios hay que saber
             * el rango dinámico de entrada del conversor A/D.
             */
            void SetAmplitude(double amplitude);


      private:

            /**
             *
             */
            struct AdcProperties {
                  double input_range; // rango de entrada, correspondiente a una muetra 1.0, en voltios
                  int sampling_frequency; // frecuencia de muestreo, en hercios
            };

            /**
             *
             */
            struct SignalProperties {
                  double rms_amplitude;
                  double main_harmonic;
                  double thd;
            };


            /**
             *
             */
            struct Buffer {
                  Buffer(int msize) {
                        size= msize;
                        data = new double[msize];    
                  }
    
                  ~Buffer() {
                        delete[] data;
                  }
    
                  int size;
                  double* data;
            };



            // Indica si el objeto se ha inicializado mediante Init()
            bool initialized_;

            // XYZ
            std::string device_;
      
      };

}

#endif // THDANALYZER_THD_ANALYZER_H_


