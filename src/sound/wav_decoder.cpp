/*
    GHL - Game Helpers Library
    Copyright (C)  Andrey Kunitsyn 2009

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Andrey (AndryBlack) Kunitsyn
    blackicebox (at) gmail (dot) com
*/

#include "wav_decoder.h"
#include "ghl_data_stream.h"
#include <cstring>


namespace GHL
{

	/** 
	 * WAVE files decoder
	 * file format description was found at http://www.sonicspot.com/guide/wavefiles.html
	 */

	WavDecoder::WavDecoder()
	{
	}
	
	WavDecoder::~WavDecoder()
	{
	}

	bool CheckWavStream(DataStream* ds);
	
	
	bool WavDecoder::Init(DataStream* ds)
	{
		if (!CheckWavStream(ds)) return false;
		/// readed shunk header
		m_ds = ds;

		::memset(&m_format,0,sizeof(m_format));

		//std::cout << "WAV: init" << std::endl;
		
		/// read shunks
		while (!ds->Eof())
		{
			Byte chunk_name[4];
			if (ds->Read(chunk_name,4)!=4) break;
			UInt32 chunk_size = 0;
			if (ds->Read(reinterpret_cast<Byte*>(&chunk_size),4)!=4) break;
			
			//// "fmt "
			if (::strncmp(reinterpret_cast<const char*>(chunk_name),"fmt ",4)==0) 
			{
				//std::cout << "WAV: found chunk 'fmt '" << std::endl;

				if (chunk_size<16) return false;
				if (ds->Read(reinterpret_cast<Byte*>(&m_format),sizeof(m_format))!=sizeof(m_format)) return false;
				//endian_fix(&m_format);
				/// uncompressed PCM
				if (m_format.compression_code!=1) return false;
				if (m_format.num_channels!=1 && m_format.num_channels!=2) return false;
				if (m_format.bits_per_sample!=8 && m_format.bits_per_sample!=16) return false;
				if (m_format.num_channels == 1) {
					if (m_format.bits_per_sample==8)
						m_type = SAMPLE_TYPE_MONO_8;
					else if (m_format.bits_per_sample==16)
						m_type = SAMPLE_TYPE_MONO_16;
				} else if (m_format.num_channels == 2) {
					if (m_format.bits_per_sample==8)
						m_type = SAMPLE_TYPE_STEREO_8;
					else if (m_format.bits_per_sample==16)
						m_type = SAMPLE_TYPE_STEREO_16;
				}
				m_freq = m_format.sample_rate;
				m_ds->Seek(chunk_size-UInt32(sizeof(m_format)),F_SEEK_CURRENT);
			} 
			/// "data"
			else if (::strncmp(reinterpret_cast<const char*>(chunk_name),"data",4)==0) 
			{
				//std::cout << "WAV: found chunk 'data'" << std::endl;
				m_samples+=chunk_size/m_format.block_align;
				m_ds->Seek(chunk_size,F_SEEK_CURRENT);
			}
			/// another
			else
			{
				//std::cout << "WAV: skip shunk '"<<chunk_name[0]<<chunk_name[1]<<chunk_name[2]<<chunk_name[3]<<"' "<< chunk_size << " bytes" << std::endl;
				/// skip
				m_ds->Seek(chunk_size,F_SEEK_CURRENT);
			}
		}
		m_ds->Seek(4+4+4,F_SEEK_BEGIN);
		m_unreaded = 0;
		return m_samples!=0 && m_type!=SAMPLE_TYPE_UNKNOWN && m_freq!=0;
	}
	
	UInt32 WavDecoder::Decode(Byte* buf,UInt32 samples)
	{
		if (!m_ds) return 0;
		UInt32 readed = 0;
		if (m_unreaded)
		{
			UInt32 size = samples > m_unreaded ? m_unreaded : samples;
			UInt32 read = m_ds->Read(buf,size*m_format.block_align);
			buf+=read;
			read/=m_format.block_align;
			samples-=read;
			m_unreaded-=read;
			readed+=read;
		}
		while (!m_ds->Eof() && samples)
		{
			Byte chunk_name[4];
			if (m_ds->Read(chunk_name,4)!=4) break;
			UInt32 chunk_size = 0;
			if (m_ds->Read(reinterpret_cast<Byte*>(&chunk_size),4)!=4) break;
			/// "data"
			if (::strncmp(reinterpret_cast<const char*>(chunk_name),"data",4)==0) 
			{
				m_unreaded+=chunk_size/m_format.block_align;
				while (m_unreaded && samples && !m_ds->Eof())
				{
					UInt32 size = samples > m_unreaded ? m_unreaded : samples;
					UInt32 read = m_ds->Read(buf,size*m_format.block_align);
					buf+=read;
					read/=m_format.block_align;
					samples-=read;
					m_unreaded-=read;
					readed+=read;
				}
			} else 
			{
				/// skip another chunks
				m_ds->Seek(chunk_size,F_SEEK_CURRENT);
			}
		}
		return readed;
	}
	void WavDecoder::Reset()
	{
		m_ds->Seek(4+4+4,F_SEEK_BEGIN);
		m_unreaded = 0;
	}

	
	SoundDecoder* CreateWavDecoder(DataStream* ds) {
		WavDecoder* dec = new WavDecoder();
		if (!dec->Init(ds)) {
			delete dec;
			return 0;
		}
		return dec;
	}
	bool CheckWavStream(DataStream* ds)
	{
		if (!ds) return false;
		ds->Seek(0,F_SEEK_BEGIN);
		Byte aChunkType[4];	
		UInt32 aChunkSize;
		if (ds->Read(aChunkType,4)!=4) return false;
		if (::strncmp(reinterpret_cast<const char*>(aChunkType), "RIFF",4) != 0)
			return false;
		if (ds->Read(reinterpret_cast<Byte*>(&aChunkSize),4)!=4) return false;
		if (ds->Read(aChunkType,4)!=4) return false;
		if (::strncmp(reinterpret_cast<const char*>(aChunkType), "WAVE",4) != 0)
			return false;
		/// ok it is a WAVE file
		return true;
	}

}