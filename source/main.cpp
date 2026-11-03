#include <3ds.h>
#include <citro2d.h>
#include "storage.h"
#include "network.h"
#include "launch.h"
#include "update_source.h"
#include "i18n.h"
#include "version.h"
#include <sys/stat.h>
#include <cstdio>
#include <cmath>
#include <unistd.h>

extern "C" { u32 __stacksize__ = 0x40000; }

static C3D_RenderTarget *top,*bottom;
static C2D_TextBuf textBuf;
static C2D_Font font;
static C2D_SpriteSheet mascot;
static Storage store("sdmc:/3ds");
static bool healthy=true;
static int selected=0;
static std::string notice="Prêt pour l'aventure.";
static const char* languageFile="sdmc:/3ds/PokeSurvivorLauncher/language.txt";
static const char* config="sdmc:/3ds/PokeSurvivorLauncher/config.ini";

static u32 rgb(int r,int g,int b,int a=255){return C2D_Color32(r,g,b,a);}
static u32 ink(){return rgb(27,21,40);}static u32 gold(){return rgb(255,210,78);}static u32 white(){return rgb(244,246,255);}
static void rect(float x,float y,float w,float h,u32 c,float z=.25f){C2D_DrawRectSolid(x,y,z,w,h,c);}
static void text(const std::string&s,float x,float y,float scale,u32 color,bool center=false,float wrap=0){
    if(scale<.45f)scale=.45f;
    auto localized=tr(s);
    C2D_Text t;C2D_TextFontParse(&t,font,textBuf,localized.c_str());C2D_TextOptimize(&t);float w,h;
    if(center){C2D_TextGetDimensions(&t,scale,scale,&w,&h);x-=w/2;}
    if(wrap)C2D_DrawText(&t,C2D_WithColor|C2D_WordWrap,floorf(x),floorf(y),.9f,scale,scale,color,wrap);
    else C2D_DrawText(&t,C2D_WithColor,floorf(x),floorf(y),.9f,scale,scale,color);
}
static void panel(float x,float y,float w,float h,u32 border){
    rect(x+2,y,w-4,h,ink(),.08f);rect(x,y+2,w,h-4,ink(),.08f);
    rect(x+2,y+1,w-4,h-2,border,.1f);rect(x+1,y+2,w-2,h-4,border,.1f);
    C2D_DrawRectangle(x+3,y+3,.2f,w-6,h-6,rgb(34,62,129),rgb(34,62,129),rgb(15,30,72),rgb(15,30,72));
    rect(x+3,y+3,w-6,1,rgb(255,255,255,45),.21f);
}
static void background(int width){C2D_DrawRectangle(0,0,0,width,240,rgb(10,20,48),rgb(10,20,48),rgb(26,48,104),rgb(26,48,104));}
static void icon(int which,float x,float y,u32 c){
    if(which==0){for(int i=0;i<6;i++)rect(x+i*3,y+i*2,3,24-i*4,c);}
    else if(which==1){rect(x+9,y,6,15,c);for(int i=0;i<4;i++)rect(x+3+i*3,y+10+i*3,18-i*6,3,c);rect(x,y+25,24,3,c);rect(x,y+20,3,7,c);rect(x+21,y+20,3,7,c);}
    else{rect(x+6,y+3,18,4,c);rect(x+20,y+6,4,16,c);rect(x+6,y+21,18,4,c);rect(x+3,y+16,4,8,c);for(int i=0;i<4;i++)rect(x+i*3,y+9-i*3,3,3+i*6,c);}
}
static void begin(C3D_RenderTarget*t){C2D_TargetClear(t,rgb(10,20,48));C2D_SceneBegin(t);}
static void drawTop(){
    background(400);
    // Pixel landscape and the same Pikachu sprite and Pixelify font as the game.
    for(int i=0;i<12;i++){int x=(i*43)%400,h=15+(i*13)%35;rect(x,174-h,28,h,rgb(17,42,73),.02f);rect(x+6,164-h,16,10,rgb(17,42,73),.02f);}
    rect(0,174,400,66,rgb(15,34,53),.03f);rect(0,174,400,3,rgb(61,105,90),.04f);
    for(int i=0;i<25;i++)rect((i*53)%400,184+(i*17)%50,3,3,rgb(48,77,77),.05f);
    text("POKÉ",200,10,.9f,gold(),true);text("SURVIVOR",200,39,1.05f,white(),true);
    text("PORT POKÉSURVIVOR 3DS",200,79,.45f,rgb(156,187,229),true);
    text("DE DAN FRANZA",200,96,.45f,gold(),true);
    if(mascot){
        int frame=(osGetTime()/150)%4;
        C2D_Image img=C2D_SpriteSheetGetImage(mascot,0);
        Tex3DS_SubTexture st{};st.width=28;st.height=28;
        st.left=float(frame*28)/128;st.right=float(frame*28+28)/128;
        st.top=1;st.bottom=1.f-28.f/128;img.subtex=&st;
        C2D_DrawImageAt(img,165,114,.5f,nullptr,2.5f,2.5f);
    }
    panel(16,187,368,39,rgb(94,127,188));
    text("VERSION INSTALLÉE",28,193,.45f,rgb(159,184,229));
    std::string v=exists(store.game())?store.version():"Jeu non installé";
    text(v,28,205,v.size()>30?.45f:.40f,white());
    text(trf("Lanceur {0}",{LAUNCHER_VERSION}),320,229,.4f,rgb(159,184,229),true);
}
static void frame(const std::string&heading="",const std::string&body="",float progress=-1,bool confirm=false){
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);C2D_TextBufClear(textBuf);begin(top);drawTop();begin(bottom);background(320);
    if(heading.empty()){
        panel(12,5,296,26,selected==3?gold():rgb(94,127,188));
        text(trf("Langue : {0}",{languageName(currentLanguage())}),155,11,.48f,white(),true);
        text("X",290,11,.48f,gold(),true);
        const char*labels[]={"Jouer",exists(store.game())?"Mettre à jour":"Installer le jeu","Restaurer la version précédente"};
        const char*desc[]={"Reprendre l'aventure","Vérifier les versions sur GitHub","Conserver la sauvegarde actuelle"};
        for(int i=0;i<3;i++){
            bool enabled=healthy&&(i==1||exists(i==0?store.game():store.backup()));
            float y=39+i*52;u32 color=!enabled?rgb(88,101,128):i==selected?gold():rgb(112,149,220);
            panel(12,y,296,46,color);icon(i,24,y+10,color);
            text(labels[i],62,y+7,i==2?.52f:.56f,color);
            text(desc[i],62,y+28,.45f,enabled?rgb(163,191,229):rgb(88,101,128));
            if(i==selected)text("▶",287,y+15,.40f,color,true);
        }
        text(notice,14,196,.45f,white(),false,292);
        text("Croix : choix  A : valider  START : quitter",160,224,.45f,rgb(159,184,229),true);
    }else{
        panel(10,15,300,208,gold());text(heading,24,29,.51f,gold());text(body,24,64,.40f,white(),false,270);
        if(progress>=0){rect(24,164,272,13,ink());rect(26,166,268*progress,9,gold());text(std::to_string(int(progress*100))+" %",160,182,.42f,white(),true);}
        if(confirm){
            panel(24,193,130,24,gold());panel(166,193,130,24,rgb(112,149,220));
            text("A Confirmer",89,198,.45f,gold(),true);
            text("B Annuler",231,198,.45f,white(),true);
        }else{
            panel(24,193,272,24,rgb(112,149,220));
            text(progress>=0?"B Annuler":"A Continuer",160,198,.45f,white(),true);
        }
    }
    C3D_FrameEnd(0);
}
static bool dialog(const std::string&h,const std::string&b,bool confirm=false){
    while(aptMainLoop()){
        hidScanInput();u32 k=hidKeysDown();frame(h,b,-1,confirm);
        if(k&KEY_A)return true;
        if(k&(KEY_B|KEY_START))return false;
        if(k&KEY_TOUCH){touchPosition p;hidTouchRead(&p);if(p.py>=193&&p.py<217&&p.px>=24&&p.px<296)return !confirm||p.px<154;}
    }
    return false;
}
static void languageMenu(){
    int choice=static_cast<int>(currentLanguage());
    while(aptMainLoop()){
        hidScanInput();u32 down=hidKeysDown();
        if(down&(KEY_B|KEY_START))return;
        if(down&KEY_DUP)choice=(choice+LANGUAGE_COUNT-1)%LANGUAGE_COUNT;
        if(down&KEY_DDOWN)choice=(choice+1)%LANGUAGE_COUNT;
        bool accept=down&KEY_A;
        if(down&KEY_TOUCH){
            touchPosition p;hidTouchRead(&p);
            for(int i=0;i<LANGUAGE_COUNT;i++)if(p.px>=18&&p.px<302&&p.py>=39+i*32&&p.py<68+i*32){choice=i;accept=true;}
        }
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);C2D_TextBufClear(textBuf);
        begin(top);drawTop();begin(bottom);background(320);
        text("Choisir la langue",160,11,.58f,gold(),true);
        for(int i=0;i<LANGUAGE_COUNT;i++){
            float y=39+i*32;bool focused=i==choice;
            panel(18,y,284,29,focused?gold():rgb(112,149,220));
            text(languageName(static_cast<Language>(i)),42,y+6,.55f,focused?gold():white());
            if(i==static_cast<int>(currentLanguage()))text("★",277,y+6,.5f,gold(),true);
        }
        text("A Valider • B Retour",160,215,.45f,rgb(159,184,229),true);
        C3D_FrameEnd(0);
        if(accept){
            setLanguage(static_cast<Language>(choice));
            if(!saveLanguagePreference(languageFile))dialog("Langue non enregistrée","Impossible d'enregistrer la langue sur la carte SD.");
            return;
        }
    }
}

static bool progress(const std::string&title,uint64_t done,uint64_t total){
    if(!aptMainLoop())return false;
    hidScanInput();char b[120];snprintf(b,sizeof b,tr("%.1f / %.1f Mo\nNe pas retirer la carte SD."),done/1048576.,total/1048576.);
    frame(title,b,total?fminf(1,float(done)/total):0);if(hidKeysDown()&(KEY_B|KEY_START))return false;
    if(hidKeysDown()&KEY_TOUCH){touchPosition p;hidTouchRead(&p);if(p.py>=193&&p.py<217&&p.px>=24&&p.px<296)return false;}
    return true;
}
static bool spaceFor(uint64_t bytes,std::string&e){FS_ArchiveResource s{};Result r=FSUSER_GetSdmcArchiveResource(&s);if(R_FAILED(r)){e="Impossible de vérifier l'espace libre.";return false;}uint64_t free=uint64_t(s.clusterSize)*s.freeClusters;if(free<bytes+1024*1024){e="Espace SD insuffisant : prévoir la taille du jeu + 1 Mo libre.";return false;}return true;}
static std::string manifestUrl(){
    auto previous=readText(config);auto url=resolveUpdateSource(previous);
    if(!url.empty()&&previous!=std::string("manifest_url=")+url+"\n")
        writeText(config,std::string("manifest_url=")+url+"\n");
    return url;
}
static void update(){
    std::string e;auto url=manifestUrl();if(!secureUrl(url)){dialog("Configuration invalide","Vérifier manifest_url dans /3ds/PokeSurvivorLauncher/config.ini.");return;}
    std::string manifest="sdmc:/3ds/PokeSurvivorLauncher/update.ini";
    frame("Recherche d'une version","Connexion HTTPS à GitHub...");
    if(!fetch(url,manifest,8192,0,e,[](uint64_t a,uint64_t b){return progress("Recherche d'une version",a,b);})){dialog("Mise à jour impossible",e);return;}
    Release r;auto metadata=readText(manifest);
    bool parsed=metadata.find_first_not_of(" \t\r\n")!=std::string::npos&&metadata[metadata.find_first_not_of(" \t\r\n")]=='{'?parseGitHubFile(metadata,r,e):parseRelease(metadata,r,e);
    if(!parsed){dialog("Manifeste refusé",e);return;}
    if(exists(store.game())&&store.version()==r.version){
        frame("Vérification du jeu","Contrôle de la version installée...");
        if(verifyReleaseFile(store.game(),r)){
            dialog("Jeu à jour",trf("La version {0} est déjà installée.",{r.version}));return;
        }
    }
    char sizeLabel[48];snprintf(sizeLabel,sizeof sizeLabel,tr("%.1f Mo"),r.size/1048576.);
    if(!dialog("Nouvelle version",trf("{0} • {1}\nInstaller cette version ?\nLa sauvegarde sera conservée.",{r.version,sizeLabel}),true))return;
    if(!spaceFor(r.size,e)){dialog("Carte SD",e);return;}
    if(!fetch(r.url,store.partial(),r.size,r.size,e,[](uint64_t a,uint64_t b){return progress("Téléchargement du jeu",a,b);})){dialog("Téléchargement arrêté",e);return;}
    frame("Vérification","Contrôle de l’intégrité du jeu...");
    if(!verifyReleaseFile(store.partial(),r)){remove(store.partial().c_str());dialog("Fichier refusé","Intégrité incorrecte. Le jeu installé est conservé.");return;}
    if(rename(store.partial().c_str(),store.staged().c_str())){dialog("Erreur SD","Impossible de préparer le fichier téléchargé.");return;}
    frame("Installation","Ne pas éteindre la console.\nConservation de la version précédente...");
    if(!store.install(r.version,e)){healthy=store.recover(e);dialog("Installation interrompue",e.empty()?"État récupéré. Réessayer la mise à jour.":e);return;}
    selected=0;notice="Mise à jour installée. À toi de jouer !";dialog("Mise à jour terminée",trf("Version {0} installée.\nLa sauvegarde est conservée.",{r.version}));
}
int main(){
    gfxInitDefault();romfsInit();C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);C2D_Init(C2D_DEFAULT_MAX_OBJECTS);C2D_Prepare();
    top=C2D_CreateScreenTarget(GFX_TOP,GFX_LEFT);bottom=C2D_CreateScreenTarget(GFX_BOTTOM,GFX_LEFT);
    textBuf=C2D_TextBufNew(4096);font=C2D_FontLoad("romfs:/pixel.bcfnt");mascot=C2D_SpriteSheetLoad("romfs:/pikachu.t3x");
    mkdir("sdmc:/3ds",0777);mkdir("sdmc:/3ds/PokeSurvivorLauncher",0777);
    if(!loadLanguagePreference(languageFile))languageMenu();
    if(!exists(config))writeText(config,std::string("manifest_url=")+defaultUpdateUrl()+"\n");
    std::string e;healthy=store.recover(e);if(!exists(store.game()))selected=1;if(!healthy){notice="Récupération nécessaire.";dialog("Erreur de récupération",e);}
    // Only abandoned downloads are removed. Transaction files are handled above.
    if(healthy){remove(store.partial().c_str());remove(store.staged().c_str());}
    while(aptMainLoop()){
        hidScanInput();u32 down=hidKeysDown();if(down&KEY_START)break;
        if(down&KEY_X){languageMenu();continue;}
        if(down&KEY_DUP)selected=(selected+3)%4;
        if(down&KEY_DDOWN)selected=(selected+1)%4;
        bool activate=down&KEY_A;
        if(down&KEY_TOUCH){touchPosition p;hidTouchRead(&p);for(int i=0;i<3;i++)if(p.px>=12&&p.px<308&&p.py>=39+i*52&&p.py<85+i*52){selected=i;activate=true;}}
        if(down&KEY_TOUCH){touchPosition p;hidTouchRead(&p);if(p.px>=12&&p.px<308&&p.py>=5&&p.py<31){selected=3;activate=true;}}
        frame();if(!activate)continue;
        if(selected==3){languageMenu();continue;}
        if(!healthy)continue;
        if(selected==0&&exists(store.game())){if(!valid3dsx(store.game())){dialog("Jeu invalide","Réinstaller le jeu avec Mettre à jour.");continue;}if(prepareLaunch(store.game(),e))break;dialog("Lancement impossible",e);}
        else if(selected==1)update();
        else if(selected==2&&exists(store.backup())){
            if(!dialog("Restaurer le jeu", trf("Revenir à la version {0} ?\nProgression conservée. Les nouveautés récentes seront indisponibles.",{tr(store.backupVersion())}),true))continue;
            struct stat s{};if(stat(store.backup().c_str(),&s)||!spaceFor(s.st_size,e)){dialog("Restauration impossible",e);continue;}
            if(store.restore(e,[](uint64_t a,uint64_t b){return progress("Copie de l'ancienne version",a,b);})){notice="Version précédente restaurée.";selected=0;dialog("Restauration terminée","Votre sauvegarde a été conservée.");}
            else{healthy=store.recover(e);dialog("Restauration interrompue",e);}
        }
    }
    if(mascot)C2D_SpriteSheetFree(mascot);
    if(font)C2D_FontFree(font);
    C2D_TextBufDelete(textBuf);C2D_Fini();C3D_Fini();romfsExit();gfxExit();return 0;
}
