system_audio_t *system_audio(){
  return OFFSETLESS(this, system_audio_t, Out);
}

f32_t InternalVolume = 1;
f32_t Volume = 1;

TH_id_t thid;

IXAudio2 *ctx = NULL;
IXAudio2MasteringVoice *MasterVoice = NULL;
IXAudio2SourceVoice *SourceVoice;

static void *_thread_func(void *p) {
  auto system_audio = (system_audio_t *)p;
  auto This = &system_audio->Out;

  while(1){
    f32_t frames[_constants::CallFrameCount * _constants::ChannelAmount] = {0};

    system_audio->Process._DataCallback(frames);

    f32_t Volume = This->Volume * This->InternalVolume;

    for(uint32_t i = 0; i < _constants::CallFrameCount * _constants::ChannelAmount; i++){
      frames[i] *= Volume;
    }

    XAUDIO2_BUFFER xabuf = {0};
    xabuf.AudioBytes = _constants::CallFrameCount * _constants::ChannelAmount * sizeof(f32_t);
    xabuf.pAudioData = (uint8_t *)frames;
    xabuf.Flags = XAUDIO2_END_OF_STREAM;
    // xabuf.pContext = (void *)This; TODO

    while(1){
      XAUDIO2_VOICE_STATE state;
      This->SourceVoice->GetState(&state);
      if (state.BuffersQueued < 2) {
        break;
      }
      This->SourceVoice->FlushSourceBuffers();
    }

    HRESULT hr = This->SourceVoice->SubmitSourceBuffer(&xabuf);
    if(FAILED(hr)){
      fan::throw_error("xaudio2", __LINE__);
    }
  }

  return 0;
}

sint32_t Open(){
  HRESULT hr;


  hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
  if (FAILED(hr)) {
    fan::throw_error("xaudio2", __LINE__);
    return 1;
  }

  hr = XAudio2Create(&ctx, 0, XAUDIO2_DEFAULT_PROCESSOR);
  if(FAILED(hr)){
    fan::throw_error("xaudio2", __LINE__);
    return 1;
  }

  hr = ctx->CreateMasteringVoice(&MasterVoice);
  if(FAILED(hr)){
    fan::throw_error("xaudio2", __LINE__);
    return 1;
  }

  WAVEFORMATEX waveFormat;
  waveFormat.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
  waveFormat.nChannels = _constants::ChannelAmount;
  waveFormat.nSamplesPerSec = _constants::opus_decode_sample_rate;
  waveFormat.wBitsPerSample = 32;
  waveFormat.nBlockAlign = waveFormat.nChannels * waveFormat.wBitsPerSample / 8;
  waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
  waveFormat.cbSize = 0;

  hr = ctx->CreateSourceVoice(
    &SourceVoice,
    &waveFormat
  );
  if(FAILED(hr)){
    fan::throw_error("xaudio2", __LINE__);
    return 1;
  }

  SourceVoice->Start(0);

  this->thid = TH_open((void *)_thread_func, system_audio());

  return 0;
}
void Close(){
  __abort();
  this->SourceVoice->DestroyVoice();
  this->MasterVoice->DestroyVoice();
  this->ctx->Release();
  CoUninitialize();
}

void SetVolume(f32_t Volume) {
  fan::throw_error_impl();
  //__atomic_store(&this->Volume, &Volume, __ATOMIC_RELAXED);
}
f32_t GetVolume() {
  f32_t r;
  fan::throw_error_impl();
 // __atomic_store(&r, &this->Volume, __ATOMIC_RELAXED);
  return r;
}

void Pause() {
  fan::throw_error("TODO not yet");
}
void Resume() {
  fan::throw_error("TODO not yet");
}
