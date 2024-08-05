#ifndef _CFG_H_
#define _CFG_H_

//#define DEBUG_MODEL
//#define _MS_STORE_

#define APP_NAME "Parlor-Board AI"

#ifndef _MS_STORE_
#define TEXTURE_PATH L"Textures\\"
#define SHADER_PATH L"Shaders\\"
#define MODELS_PATH L"Models\\"
#else
#define TEXTURE_PATH L"\\ProgramData\\rezek\\"
#define SHADER_PATH L"\\ProgramData\\rezek\\"
#define MODELS_PATH L"\\ProgramData\\rezek\\"
#endif

#endif /* _CFG_H_ */
