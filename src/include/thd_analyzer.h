// Hi Emacs, this is -*- mode: c++; tab-width: 6; indent-tabs-mode: nil; c-basic-offset: 6 -*-
#ifndef THDANALYZER_THD_ANALYZER_H_
#define THDANALYZER_THD_ANALYZER_H_

#include <stdint.h>
#include <string>
#include <alsa/asoundlib.h>
#include <pthread.h>


namespace thd_analyzer {


      class ThdAnalyzer;


      // Máscara espectral fija. TODO: Hacerla configurable.
      class SpectrumMask {
      public:
            
            SpectrumMask(int sampling_rate, int fft_size);

            ~SpectrumMask();

            // Número de frecuencias en el espectro actual que TRASPASAN la máscara
            int    error_count;

            // Primera frecuencia que traspasa la máscara
            double first_trespassing_frequency;

            // Valor de dicha frecuencia
            double first_trespassing_value;

            // Última frecuencia analógica (la de valor más alto) que traspasa la máscara
            double last_trespassing_frequency;
            
            // Valor de dicha frecuencia
            double last_trespassing_value;

      private:
            
            friend class ThdAnalyzer;   

            // La máscar no es totalmente rígida, se puede desplazar arriba y abajo en el eje de amplitud.
            double vertical_offset;

            // Valores de la máscara, es una array de [size] elementos
            double* value;
            int size;

            int fs;
      };


      /**
       * Analizador de espectro en tiempo real para señales de audio.
       *
       * Analiza N señales de audio provenientes de un dispositivo ADC multicanal con interfaz Linux ALSA . El ADc del
       * sistema puede ser, como digo, multicanal de N canales. Lo más típico es que sea estéreo con lo cual N = 2 (L y
       * R) pero este software está preparado para trabajar en general con N canales incluyendo por supuesto audio
       * monoaural (N = 1).
       *
       * Realiza las siguientes medidas:
       *
       * - Estimación de la densidad espectral de potencia por el método del periodograma (FFT al cuadrado).
       * - Localización de la frecuencia cuya potencia es máxima en el espectro.
       * - Relación señal a ruido más interferencias (SNRI )
       + - comprueba si un espectro se ajusta a una máscara dada o no, y contabiliza el número de frecuencias que están 
       *   rebasando la máscara.
       *
       * Con las medidas anteriores podemos usar este componente, por ejemplo, para ver el espectro de la señal o para
       * detectar la presencia o no de tonos enterrados en ruido y estimar su frecuencia y su amplitud.
       *
       */
      class ThdAnalyzer {
      public:

            /**
             * Constructor.
             * 
             *
             * @param capture_device El dispositivo ALSA que captura de muestras de audio, por ejemplo "default" es el
             * dispositivo predeterminado del sistema y casi siempre representa la entrada de linea o micrófono, otros
             * dispositivos son por ejemplo "hw:0,0", "hw:1,0", "plughw:0,0", etc. Todo esto depende del sistema en
             * concreto, pruebe a ejecutar el programa "arecord -l" para ver una lista de dispositivos. Muy importante:
             * ajuste primero el volumen de grabación y active el capturador (unmute) con el programa alsamixer.
             *
             * @param sampling_rate Frecuencia de muestreo (Fs) del ADC, en Hz. Cuidado: No siempre funciona lo de
             * ajustar la frecuencia de muestreo, en mi PC, no sé aun el porqué, independientemente de lo que le pongas
             * aquí la Fs es siempre 192000 Hz. Valores típicos son: 44100 Hz, 48000 Hz, 96000 Hz y 192000 Hz. Por otro
             * lado, la resolución de las muestras es fijo de 16 bits con signo (Little Endian INT16).
             *
             * @param log2_block_size Logaritmo en base 2 del número de puntos que se calculan en elespectro. Si aquí se
             * pasa por ejemplo 10 significa que se van a calcular 2^10 = 1024 puntos del espectro. Cuantos más puntos
             * más resolución espectral tendremos pero también más carga computacional. Valores típicos son 10, 11 o 12.
             * 
             */
            ThdAnalyzer(const char* capture_device, int sampling_rate, int log2_block_size);


            /**
             * Destructor. 
             *             
             */
            ~ThdAnalyzer();


            /**
             * Inicialización.
             *
             * Configura el dispositivo ADC de captura y pone en marcha el hilo de procesado de señal.
             */
            int Init();



            /**
             * TODO: Da comienzo a la captura de muestras de audio y al análisis de las señales. Mientras el análisis esté en
             * marcha podremos llamar a las funciones Frequency() etc para obtener las medidas.
             */
            //int Start();

            // bool IsRunning();

            /**
             * TODO: Detiene la captura de muestras de audio y el análisis. Las medidas no cambiarán, se mantendrán en el
             * último estado antes de llamar a Stop().
             */
            //int Stop();

            /**
             * TODO: Vacía los búferes de muestras, contadores, etc en genral todo el estado interno.
             */
            //int Reset();


            /**
             * Frecuencia de muestreo en hercios (Hz) que está usando el analizador.
             */
            int SamplingFrequency() const;

            /**
             * Número de puntos de la DFT, que será siempre una potencia de 2. 
             */
            int DftSize() const;
            
            /**
             * Resolución en frcuencia que tiene este analizador, expresado en Hz.
             *
             * No es posible distinguir dos tonos que estén separados menos que esta cantidad, que es simplemente
             * SamplingFrequency() / DftSize()
             */
            double AnalogResolution() const;

            /**
             * Devuelve la frecuencia analógica en hercios (Hz) correpondiente al índice |frequency_index|. 
             * 
             * La frecuencia analógica es |frequency_index| * (Fs / N) donde Fs es la frecuencia de muestreo y N es el
             * número de puntos de la FFT, que se obtiene con DftSize()
             *
             * @return La frecuencia analógica en hertzios (Hz) correpondiente al índice suministrado.
             */
            double AnalogFrequency(int frequency_index);

            /**
             * Escribe en disco un fichero de texto con los puntos del espectro listo para visualizar con gnuplot,
             * octave, matlab, etc. Tiene DftSize() filas y N+1 columnas, siendo N el número de canales. La primera
             * columna es la frecuencia analógica y las demás los valores del espectro.
             */
            int GnuplotFileDump(std::string file_name);


            // ----- MEDIDAS -------------------------------------------------------------------------------------------

            /**
             * Devuelve la máscara espectral que se está usando y los contadores de errores asociados.
             */
            const SpectrumMask* Mask(int channel) const { return channel_[channel].mask; }


            /**
             * Devuelve el índice de frecuencia, es decir, el punto de la FFT cuyo módulo es el máximo absoluto. Los
             * índices empiezan en 0, van desde 0 hasta (BlockSize() - 1). Para obtener la frecuencia analógica que
             * corresponde a este punto llame a AnalogFrequency().
             *
             * @param channel Número de canal. En un dispositivo de captura estéreo el 0 es el canal izquierdo y el 1 es
             * el canal derecho.
             */
            int FindPeak(int channel);
                      

            /**
             * Estimación de la relación señal a ruido más interferente (SNRI).
             *
             * Esta estimación es bastante simplista, considera que la banda de frecuencia en la que está la señal (y
             * solo está ella, ahí no hay ruido ni inteferencia) es (f1, f2) Hz, lo cual no tiene porqué ser cierto.
             */
            double SNRI(int channel, double f1, double f2);
 

            /**
             * Densidad espectral de potencia correspondiente a la frecuencia |frequency_index|. 
             * La unidad es vatios / (radian / muestra)
             *
             * @param channel El número de canal, es un dispositivo estéreo 0 es el izquierdo y 1 el derecho.
             * @param frequency_index Índice de la frecuencia en la que se evaluará la densidad espectral de potencia.
             *
             */
            double PowerSpectralDensity(int channel, int frequency_index);

            /**
             * Lo mismo que PowerSpectralDensity() pero expresado en decibelios, es decir, aplicando
             * 10*log10(x). Expresado en dB la cantidad siempre será negativa, el máximo valor posible es 0 dB.
             *
             * @param frequency_index Índice de la frecuencia en la que se evaluará la densidad espectral de potencia.
             */
            double PowerSpectralDensityDecibels(int channel, int frequency_index);


            /**
             * Número de bloques de DftSize() muestras procesados en cada canal desde que el hilo interno de procesado
             * se puso en marcha mediante Init(). Sirve por ejemplo para comprobar que el hilo interno de procesado 
             * no se ha parado y está vivo.
             */
            int BlockCount() const;



      private:

            /**
             * Datos brutos y calculados de uno de los canales de entrada del ADC (normalmente será un ADC estéreo y
             * tendrá por tanto dos canales, L y R)
             */
            struct Channel {

                  // pthread_mutex_lock/unlock 
                  pthread_mutex_t lock;

                  // TODO: size
                  
                  // |block_size_| muestras convertidas a coma flotante y normalizadas en el intevalo real [-1.0, 1.0)
                  double* data;

                  // |block_size_| muestras de la densidad espectral de potencia estimada |X(k)|^2
                  double* pwsd;

                  // Valor del máximo del espectro
                  double peakv;

                  // Frecuencia en la que se produce ese máximo espectral.
                  int peakf;

                  // Valor RSM (Root Mean Square) del bloque de muestras capturado
                  //double rms;
                  SpectrumMask* mask;
            };

            // Número de canales del dispositivo ADC, lo normal es que sea estéreo: 2 canales.
            int channel_count_;

            // Parametros comunes a todos los canales

            // Tamaño de bloque en muestras. El procesamiento de señal
            // se hace por bloques de muestras, no muestra a muestra, que seria muy ineficiente.
            int block_size_;

            // log2(tamaño del bloque)
            int block_size_log2_;

            // Frecuencia de muestreo
            int sample_rate_;

            // Numero de bloques procesados
            int block_count_;

            Channel* channel_;
            

           

            // Hilo
            pthread_t thread_;
            pthread_attr_t thread_attr_;

            // Indica si el objeto se ha inicializado mediante Init()
            bool initialized_;
            bool exit_thread_;

            // Dispositivo ALSA de captura
            std::string device_;

            // Representa el dispositivo ALSA de captura de audio.
            snd_pcm_t* capture_handle_;

            // Búfer en el se reciben las muestras en el formato en que las entrega el ADC, que normalmente será int16_t
            // las muestras podrán pertenecer a un solo canal o a varios intercalados. Si por ejemplo hay dos canales
            // (estereo) entonces las muestras estarán dispuestas de la forma L R L R L R L R L R...)
            int16_t* buf_data_;
            int      buf_size_;

            static void* ThreadFuncHelper(void* p);

            void* ThreadFunc();

            
            /**
             * This computes an in-place complex-to-complex FFT 
             * x and y are the real and imaginary arrays of 2^m points.
             * dir =  1 gives forward transform
             * dir = -1 gives reverse transform 
             */
            void FFT(short int dir, long m, double* x, double* y);

            /**
             * Process
             */
            int Process();
      
      };

}

#endif // THDANALYZER_THD_ANALYZER_H_



