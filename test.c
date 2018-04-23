#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "picoapi.h"
#include <alsa/asoundlib.h>

const int MEM_SIZE = 1024 * 1024 * 5;
const int BUF_SIZE = 160000;

void playback(short *buf, int len) {
    int i;
    int err;
    snd_pcm_t *playback_handle;
    snd_pcm_hw_params_t *hw_params;

    if ((err = snd_pcm_open (&playback_handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        fprintf (stderr, "cannot open audio device %s (%s)\n",
                "default",
                snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
        fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
                snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_any (playback_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
                snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_access (playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf (stderr, "cannot set access type (%s)\n",
                snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params_set_format (playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        fprintf (stderr, "cannot set sample format (%s)\n",
                snd_strerror (err));
        exit (1);
    }

    // int rate = 44100;
    int rate = 16000;
    if ((err = snd_pcm_hw_params_set_rate_near (playback_handle, hw_params, &rate, 0)) < 0) {
        fprintf (stderr, "cannot set sample rate (%s)\n",
                snd_strerror (err));
        exit (1);
    }
    printf("Playing at rate: %d\n", rate);

    if ((err = snd_pcm_hw_params_set_channels (playback_handle, hw_params, 1)) < 0) {
        fprintf (stderr, "cannot set channel count (%s)\n",
                snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_hw_params (playback_handle, hw_params)) < 0) {
        fprintf (stderr, "cannot set parameters (%s)\n",
                snd_strerror (err));
        exit (1);
    }

    snd_pcm_hw_params_free (hw_params);

    if ((err = snd_pcm_prepare (playback_handle)) < 0) {
        fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
                snd_strerror (err));
        exit (1);
    }

    if ((err = snd_pcm_writei (playback_handle, buf, len)) != len) {
        fprintf (stderr, "write to audio interface failed (%s)\n",
                snd_strerror (err));
        exit (1);
    }

    snd_pcm_close (playback_handle);
}

// Error handling.

#define CHECKED_SYS(RET)   checked_sys(system, RET, 0)
#define CHECKED_SYS_REC(RET)   checked_sys(system, RET, depth + 1)
void checked_sys(pico_System system, pico_Status ret, int depth) {
    // We are going to just assume call related to warnings don't error,
    // since missing warnings isn't the end of the world anyways
    int num_warnings = 0;
    pico_getNrSystemWarnings(
        system,
        &num_warnings
    );

    for( pico_Int32 i = 0; i < num_warnings; i++ ) {
        pico_Retstring message;
        pico_Status code;

        pico_getSystemWarning(
            system,
            i,
            &code,
            message
        );

        fprintf(stderr, "Pico warning %d: %s\n", code, message);
    }


    if( ret != 0 ) {
        // Recursive error handling may not be the best idea ever
        if( depth > 3 ) {
            fprintf(stderr, "Recursive errors\n");
            exit(1);
        }

        pico_Retstring message;

        checked_sys(
            system,
            pico_getSystemStatusMessage(
                system,
                ret,
                message
            ),
            depth + 1
        );

        fprintf(stderr, "pico error %d: %s\n", ret, message);
        exit(1);
    }
}

// This is all just copy and pasted from the above with system replaced by engine.
#define CHECKED_ENGINE(RET)   checked_engine(engine, RET, 0)
#define CHECKED_ENGINE_REC(RET)   checked_engine(engine, RET, depth + 1)
void checked_engine(pico_Engine engine, pico_Status ret, int depth) {
    // We are going to just assume call related to warnings don't error,
    // since missing warnings isn't the end of the world anyways
    int num_warnings = 0;
    pico_getNrEngineWarnings(
        engine,
        &num_warnings
    );

    for( pico_Int32 i = 0; i < num_warnings; i++ ) {
        pico_Retstring message;
        pico_Status code;

        pico_getEngineWarning(
            engine,
            i,
            &code,
            message
        );

        fprintf(stderr, "Pico warning %d: %s\n", code, message);
    }


    if( ret != 0 ) {
        // Recursive error handling may not be the best idea ever
        if( depth > 3 ) {
            fprintf(stderr, "Recursive errors\n");
            exit(1);
        }

        pico_Retstring message;

        checked_engine(
            engine,
            pico_getEngineStatusMessage(
                engine,
                ret,
                message
            ),
            depth + 1
        );

        fprintf(stderr, "pico error %d: %s\n", ret, message);
        exit(1);
    }
}

int main() {
    pico_System system;

    // Pico asks us to initialize memory, and won't otherwise allocate memory.
    // Unfortunately it doesn't tell us exactly how much memory it's going to need.
    // The documentation suggests they had a target of 200KB when creating it,
    // but it crashes at 1MB. So we just give it 5MB.

    CHECKED_SYS(pico_initialize(
        malloc(MEM_SIZE),
        MEM_SIZE,
        // Defined as an 'out' variable in the header, this initializes system
        &system
    ));

    // Resource files include the actual data needed for tts.

    // "Text analysis" lingware resource file
    pico_Resource res_ta;
    CHECKED_SYS(pico_loadResource(
        system,
        "lang/en-GB_ta.bin",
        &res_ta
    ));

    // For whatever reason the resource is sometimes referred to by name instead
    // of by handle
    pico_Retstring res_ta_name;
    CHECKED_SYS(pico_getResourceName(
        system,
        res_ta,
        res_ta_name
    ));

    printf("Loaded resource with name: '%s'\n", res_ta_name);

    // "Signal generation" lingware resource file
    pico_Resource res_sg;
    CHECKED_SYS(pico_loadResource(
        system,
        "lang/en-GB_kh0_sg.bin",
        &res_sg
    ));

    pico_Retstring res_sg_name;
    CHECKED_SYS(pico_getResourceName(
        system,
        res_sg,
        res_sg_name
    ));

    printf("Loaded resource with name: '%s'\n", res_sg_name);

    // This creates a name which we can use for later reference
    CHECKED_SYS(pico_createVoiceDefinition(
        system,
        "PicoVoice"
    ));

    // This attatches voice data to the name
    CHECKED_SYS(pico_addResourceToVoiceDefinition(
        system,
        "PicoVoice",
        res_ta_name
    ));

    CHECKED_SYS(pico_addResourceToVoiceDefinition(
        system,
        "PicoVoice",
        res_sg_name
    ));

    // Note that if you incorrectly load resourceData
    // (i.e. only load ta) this segfautls in the error
    // handling code, but gdb will show you the error
    // message if you compile everything with debugging
    // symbols.
    pico_Engine engine;
    CHECKED_SYS(pico_newEngine(
        system,
        "PicoVoice",
        &engine
    ));

    char *text = "Hello world!";

    pico_Int16 bytes_copied;
    CHECKED_ENGINE(pico_putTextUtf8(
        engine,
        text,
        strlen(text) + 1,
        &bytes_copied
    ));

    printf("bytes copied: %d/%d\n", bytes_copied, strlen(text));

    FILE *audio_f = fopen("/dev/audio", "w");

    short *buf = malloc(BUF_SIZE * 4);
    int buf_len = 0;

    pico_Int16 bytes_received = 0;
    pico_Int16 dtype = 0;
    pico_Status ret;
    do {
        ret = pico_getData(
            engine,
            buf + buf_len,
            1000,
            &bytes_received,
            &dtype
        );

        // We are assuming that we get shorts back... which
        // is currently the only supported dtype anyways but
        // we might as well check.
        if( dtype != PICO_DATA_PCM_16BIT ) {
            fprintf(stderr, "Unknown dtype\n");
            exit(1);
        }

        buf_len += bytes_received / 2;

        // The 3 * part is just to try and make sure we will have space
        // for the next batch of bytes...
        if( BUF_SIZE - buf_len - 3 * bytes_received < 0 ) {
            printf("Early finish, %d\n", buf_len);
            playback(buf, buf_len);

            buf_len = 0;
        }
    }
    while( ret == PICO_STEP_BUSY );

    playback(buf, buf_len);

    // We could cleanup here, but program termination will do it for us anyways.

    return 0;
}