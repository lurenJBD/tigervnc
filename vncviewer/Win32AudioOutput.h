/* Copyright 2022 Mikhail Kupchik
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
 * USA.
 */

#ifndef __WIN32AUDIOOUTPUT_H__
#define __WIN32AUDIOOUTPUT_H__

#include <stdint.h>

#include <windows.h>

class Win32AudioOutput {

  private:
    struct HdrInSlist {
      WAVEHDR          whdr;
      ULONGLONG        streamId;
      BOOL             starvedWhenDone;
      ULONGLONG        starvationTimestamp;
      union {
        HdrInSlist*    next;
        PVOID volatile volatileNext;
      };
    };

  public:
    static const size_t maxNetworkJitterInMillisec = 1000;

    static const uint8_t sampleFormatU8  = 0;
    static const uint8_t sampleFormatS8  = 1;
    static const uint8_t sampleFormatU16 = 2;
    static const uint8_t sampleFormatS16 = 3;
    static const uint8_t sampleFormatU32 = 4;
    static const uint8_t sampleFormatS32 = 5;

    Win32AudioOutput();
    ~Win32AudioOutput();

    bool isAvailable() const { return haveWO; }
    uint8_t getSampleFormat() const { return sampleFormat; } 
    uint8_t getNumberOfChannels() const { return numberOfChannels; }
    uint32_t getSamplingFreq() const { return samplingFreq; }
    size_t getSampleSize() const { return (numberOfChannels << (sampleFormat >> 1)); }

    bool openAndAllocateBuffer();
    bool isOpened() const { return openedWO; }

    void notifyStreamingStartStop(bool isStart);
    void addSilentSamples(size_t numberOfSamples);
    size_t addSamples(const uint8_t* data, size_t size);
    bool submitSamples();

  private:
    static ULONGLONG getCurrentTimestamp();
    static void CALLBACK waveOutCallback(HWAVEOUT hwo, UINT msg, DWORD_PTR instance,
                                         DWORD_PTR param1, DWORD_PTR param2);

    bool           haveWO, openedWO;
    uint8_t        sampleFormat, numberOfChannels;
    uint32_t       samplingFreq;
    ULONGLONG      currentStreamId;
    HWAVEOUT       handleWO;
    uint8_t*       bufPtr;
    size_t         bufTotalSize;
    size_t         bufFreeSize;
    size_t         bufUnsubmittedSize;
    size_t         bufSubmittedHead;
    size_t         bufUnsubmittedHead;
    PVOID volatile doneHdrsSlist;
    LONG volatile  hdrsInFlight;
    uint32_t       extraDelayInMillisec;
};

#endif // __WIN32AUDIOOUTPUT_H__
