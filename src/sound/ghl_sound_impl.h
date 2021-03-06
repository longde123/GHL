//
//  ghl_sound_impl.h
//  GHL
//
//  Created by Andrey Kunitsyn on 8/25/12.
//  Copyright (c) 2012 AndryBlack. All rights reserved.
//

#ifndef GHL_ghl_sound_impl_h
#define GHL_ghl_sound_impl_h

#include <ghl_sound.h>
#include "../ghl_ref_counter_impl.h"
#include "ghl_sound_decoder.h"

namespace GHL {
    
    
    class SoundEffectImpl : public RefCounterImpl<SoundEffect> {
    private:
        SampleType  m_type;
        UInt32      m_freq;
        UInt32      m_capacity;
    protected:
        void SetCapacity( UInt32 samples ) { m_capacity = samples; }
        
    public:
        SoundEffectImpl( SampleType type, UInt32 freq ) :
            m_type(type),m_freq(freq),m_capacity(0) {}
        virtual ~SoundEffectImpl() {}
        /// sample type
        virtual SampleType GHL_CALL GetSampleType() const { return m_type; }
        /// samples rate
        virtual UInt32 GHL_CALL GetFrequency() const { return m_freq; }
        /// samples amount
        virtual UInt32 GHL_CALL GetSamplesAmount() const { return m_capacity; }
        
        UInt32 GetChannels() const {
            return SoundDecoderBase::GetChannels(m_type);
        }
        UInt32 GetBits() const {
            return SoundDecoderBase::GetBps(m_type)*8;
        }
    };
    
    class SoundImpl : public Sound {
    public:
		virtual ~SoundImpl() {}
        virtual bool SoundInit() { return true; }
        virtual bool SoundDone() { return true;}
    };
}

#endif
