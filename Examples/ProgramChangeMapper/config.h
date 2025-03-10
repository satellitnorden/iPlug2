#define PLUG_NAME "Program Change Mapper"
#define PLUG_MFR "Daybreak Studio"
#define PLUG_VERSION_HEX 0x00000100
#define PLUG_VERSION_STR "0.1.0"
#define PLUG_UNIQUE_ID 'V237'
#define PLUG_MFR_ID 'Acme'
#define PLUG_URL_STR "https://iplug2.github.io"
#define PLUG_EMAIL_STR "DaybreakStudioSwe@gmail.com"
#define PLUG_COPYRIGHT_STR "Copyright 2022 Daybreak Studio"
#define PLUG_CLASS_NAME ProgramChangeMapper

#define BUNDLE_NAME "Program Change Mapper"
#define BUNDLE_MFR "Daybreak Studio"
#define BUNDLE_DOMAIN "com"

#define PLUG_CHANNEL_IO "0-0"
#define SHARED_RESOURCES_SUBPATH "ProgramChangeMapper"

#define PLUG_LATENCY 0
#define PLUG_TYPE 2
#define PLUG_DOES_MIDI_IN 1
#define PLUG_DOES_MIDI_OUT 1
#define PLUG_DOES_MPE 0
#define PLUG_DOES_STATE_CHUNKS 1
#define PLUG_HAS_UI 1
#define PLUG_WIDTH 512
#define PLUG_HEIGHT 512
#define PLUG_FPS 60
#define PLUG_SHARED_RESOURCES 0
#define PLUG_HOST_RESIZE 0

#define AUV2_ENTRY ProgramChangeMapper_Entry
#define AUV2_ENTRY_STR "ProgramChangeMapper_Entry"
#define AUV2_FACTORY ProgramChangeMapper_Factory
#define AUV2_VIEW_CLASS ProgramChangeMapper_View
#define AUV2_VIEW_CLASS_STR "ProgramChangeMapper_View"

#define AAX_TYPE_IDS 'IPI1', 'IPI2'
#define AAX_PLUG_MFR_STR "Acme"
#define AAX_PLUG_NAME_STR "ProgramChangeMapper\nIPIS"
#define AAX_DOES_AUDIOSUITE 0
#define AAX_PLUG_CATEGORY_STR "Synth"

#define VST3_SUBCATEGORY "Instrument"

#define APP_NUM_CHANNELS 2
#define APP_N_VECTOR_WAIT 0
#define APP_MULT 1
#define APP_COPY_AUV3 0
#define APP_SIGNAL_VECTOR_SIZE 64

#define PROGRAMCHANGEMAPPER_FN "Roboto-Regular.ttf"
#define PROGRAMCHANGEMAPPER_BG "Background.png"
