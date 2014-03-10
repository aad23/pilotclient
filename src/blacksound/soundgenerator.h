/*  Copyright (C) 2013 VATSIM Community / contributors
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BLACKSOUND_SOUNDGENERATOR_H
#define BLACKSOUND_SOUNDGENERATOR_H

#include "blackmisc/avselcal.h"
#include "blackmisc/audiodevice.h"
#include "blackmisc/pqtime.h"
#include <QIODevice>
#include <QThread>
#include <QDateTime>
#include <QAudioFormat>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QMediaPlayer>

namespace BlackSound
{

    /*!
     * \brief Paying simple sounds
     */
    class CSoundGenerator : public QIODevice
    {
        Q_OBJECT

    public:

        /*!
         * \brief How to play
         */
        enum PlayMode
        {
            Single,
            SingleWithAutomaticDeletion,
            EndlessLoop
        };

        /*!
         * \brief Play notification
         */
        enum Notification
        {
            NotificationError = 0,
            NotificationLogin,
            NotificationLogoff,
            NotificationTextMessage,
        };

        /*!
         * \brief Tone to be played
         */
        struct Tone
        {
            friend class CSoundGenerator;

        private:
            int m_frequencyHz; /*!< first tone's frequency, use 0 for silence */
            int m_secondaryFrequencyHz; /*!< second tone's frequency, or 0 */
            qint64 m_durationMs; /*!< How long to play (duration) */

        public:
            /*!
             * \brief Play frequency f for t milliseconds
             */
            Tone(const BlackMisc::PhysicalQuantities::CFrequency &frequency, const BlackMisc::PhysicalQuantities::CTime &duration) :
                m_frequencyHz(static_cast<int>(frequency.valueRounded(BlackMisc::PhysicalQuantities::CFrequencyUnit::Hz()))),
                m_secondaryFrequencyHz(0),
                m_durationMs(static_cast<qint64>(duration.valueRounded(BlackMisc::PhysicalQuantities::CTimeUnit::ms()))) {}

            /*!
             * \brief Play 2 frequencies f for t milliseconds
             */
            Tone(const BlackMisc::PhysicalQuantities::CFrequency &frequency, const BlackMisc::PhysicalQuantities::CFrequency &secondaryFrequency, const BlackMisc::PhysicalQuantities::CTime &duration) :
                m_frequencyHz(static_cast<int>(frequency.valueRounded(BlackMisc::PhysicalQuantities::CFrequencyUnit::Hz()))),
                m_secondaryFrequencyHz(static_cast<int>(secondaryFrequency.valueRounded(BlackMisc::PhysicalQuantities::CFrequencyUnit::Hz()))),
                m_durationMs(static_cast<qint64>(duration.valueRounded(BlackMisc::PhysicalQuantities::CTimeUnit::ms()))) {}
        };

        /*!
         * \brief Constructor
         * \param device        device
         * \param format        audio format
         * \param tones         list of Tones
         * \param mode          play once?
         * \param parent
         * \see PlayMode
         */
        CSoundGenerator(const QAudioDeviceInfo &device, const QAudioFormat &format, const QList<Tone> &tones, PlayMode mode, QObject *parent = nullptr);

        /*!
         * \brief Constructor
         * \param tones         list of Tones
         * \param mode          play once?
         * \param parent
         * \see PlayMode
         */
        CSoundGenerator(const QList<Tone> &tones, PlayMode mode, QObject *parent = nullptr);

        /*!
         * Destructor
         */
        ~CSoundGenerator();

        /*!
         * \brief Set volume
         * \param volume 0..100
         */
        void setVolume(int volume)
        {
            this->m_audioOutput->setVolume(qreal(volume / 100.0));
        }

        /*!
         * \brief Close device, buffer stays intact
         */
        void stop(bool destructor = false);

        /*!
         * \brief duration of one cycle
         */
        qint64 singleCyleDurationMs() const { return calculateDurationMs(this->m_tones); }

        /*!
         * \copydoc QIODevice::readData()
         */
        virtual qint64 readData(char *data, qint64 maxlen) override;

        /*!
         * \copydoc QIODevice::writeData()
         * \remarks NOT(!) used here
         */
        virtual qint64 writeData(const char *data, qint64 len) override;

        /*!
         * \copydoc QIODevice::bytesAvailable()
         */
        virtual qint64 bytesAvailable() const override;

        /*!
         * \copydoc QIODevice::seek()
         */
        virtual bool seek(qint64 pos) override
        {
            return this->m_endReached ? false : QIODevice::seek(pos);
        }

        /*!
         * \copydoc QIODevice::atEnd()
         */
        virtual bool atEnd() const override
        {
            return this->m_endReached ? true : QIODevice::atEnd();
        }

        /*!
         * \brief Default audio format fo play these sounds
         * \return
         */
        static QAudioFormat defaultAudioFormat();

        /*!
         * \brief Find the closest Qt device to this audio device
         * \param audioDevice   output audio device
         * \return
         */
        static QAudioDeviceInfo findClosestOutputDevice(const BlackMisc::Audio::CAudioDevice &audioDevice);

        /*!
         * \brief Play signal of tones once
         * \param volume    0-100
         * \param tones     list of tones
         * \param device    device to be used
         * \return generator used, important with SingleWithAutomaticDeletion automatically deleted
         */
        static CSoundGenerator *playSignal(qint32 volume, const QList<Tone> &tones, QAudioDeviceInfo device = QAudioDeviceInfo::defaultOutputDevice());

        /*!
         * \brief Play signal of tones once
         * \param volume    0-100
         * \param tones     list of tones
         * \param device    device to be used
         * \return generator used, important with SingleWithAutomaticDeletion automatically deleted
         */
        static CSoundGenerator *playSignalInBackground(qint32 volume, const QList<CSoundGenerator::Tone> &tones, QAudioDeviceInfo device);

        /*!
         * \brief Record the tones to a wav file, then play the wav file
         * \param volume    0-100
         * \param tones     list of tones
         * \param device    device to be used
         */
        static void playSignalRecorded(qint32 volume, const QList<CSoundGenerator::Tone> &tones, QAudioDeviceInfo device);

        /*!
         * \brief Play SELCAL tone
         * \param volume    0-100
         * \param selcal
         * \param device    device to be used
         * \see BlackMisc::Aviation::CSelcal
         */
        static void playSelcal(qint32 volume, const BlackMisc::Aviation::CSelcal &selcal, QAudioDeviceInfo device = QAudioDeviceInfo::defaultOutputDevice());

        /*!
         * \brief Play SELCAL tone
         * \param volume    0-100
         * \param selcal
         * \param audioDevice device to be used
         * \see BlackMisc::Aviation::CSelcal
         */
        static void playSelcal(qint32 volume, const BlackMisc::Aviation::CSelcal &selcal, const BlackMisc::Audio::CAudioDevice &audioDevice);

        /*!
         * \brief Play notification
         * \param volume    0-100
         * \param notification
         */
        static void playNotificationSound(qint32 volume, Notification notification);

        /*!
         * \brief One cycle of tones takes t milliseconds
         */
        BlackMisc::PhysicalQuantities::CTime oneCycleDurationMs() const
        {
            return BlackMisc::PhysicalQuantities::CTime(this->m_oneCycleDurationMs, BlackMisc::PhysicalQuantities::CTimeUnit::ms());
        }

        /*!
         * \brief Play given file
         * \param volume    0-100
         * \param file
         * \param removeFileAfterPlaying delete the file, after it has been played
         */
        static void playFile(qint32 volume, const QString &file, bool removeFileAfterPlaying);

    signals:
        /*!
         * \brief Device was closed
         * \remarks With singleShot the signal indicates that sound sequence has finished
         */
        void stopped();

    public slots:
        /*!
         * \brief Play sound, open device
         * \param volume 0..100
         * \param pull pull/push, if false push mode
         */
        void start(int volume, bool pull = true);

        /*!
         * \brief Play sound in own thread, open device
         * \remarks always push mode
         * \param volume 0..100
         */
        void startInOwnThread(int volume);

    signals:

        //! \brief Generator is stopping
        void stopping();

    private slots:
        /*!
         * \brief Push mode, timer expired
         */
        void pushTimerExpired();

    private:
        /*!
         * \brief Generate tone data in internal buffer
         */
        void generateData();

    private:
        QList<Tone> m_tones; /*! tones to be played */
        qint64 m_position; /*!< position in buffer */
        PlayMode m_playMode; /*!< end data provisioning after playing all tones, play endless loop */
        bool m_endReached; /*!< indicates end in combination with single play */
        qint64 m_oneCycleDurationMs; /*!< how long is one cycle of tones */
        QByteArray m_buffer; /*!< generated buffer for data */
        QAudioDeviceInfo m_device; /*!< audio device */
        QAudioFormat m_audioFormat; /*!< used format */
        QScopedPointer<QAudioOutput> m_audioOutput;
        QTimer *m_pushTimer; /*!< Push mode timer */
        QIODevice *m_pushModeIODevice; /*!< IO device when used in push mode */
        QThread *m_ownThread;
        static QDateTime s_selcalStarted;

        //! \brief Header for saving .wav files
        struct chunk
        {
            char        id[4];
            quint32     size;
        };

        //! \brief Header for saving .wav files
        struct RiffHeader
        {
            chunk       descriptor;     // "RIFF"
            char        type[4];        // "WAVE"
        };

        //! \brief Header for saving .wav files
        struct WaveHeader
        {
            chunk       descriptor;
            quint16     audioFormat;
            quint16     numChannels;
            quint32     sampleRate;
            quint32     byteRate;
            quint16     blockAlign;
            quint16     bitsPerSample;
        };

        //! \brief Header for saving .wav files
        struct DataHeader
        {
            chunk       descriptor;
        };

        //! \brief Header for saving .wav files
        struct CombinedHeader
        {
            RiffHeader  riff;
            WaveHeader  wave;
            DataHeader  data;
        };

        //! \brief "My" media player
        static QMediaPlayer *mediaPlayer()
        {
            static QMediaPlayer *mediaPlayer = new QMediaPlayer();
            return mediaPlayer;
        }

        //! \brief Duration of these tones
        static qint64 calculateDurationMs(const QList<Tone> &tones);

        //! \brief save buffer to wav file
        bool saveToWavFile(const QString &fileName) const;

        /*!
         * \brief Write amplitude to buffer
         * \param amplitude value -1 .. 1
         * \param bufferPointer current buffer pointer
         */
        void writeAmplitudeToBuffer(const double amplitude, unsigned char *bufferPointer);
    };
} //namespace


#endif // guard
