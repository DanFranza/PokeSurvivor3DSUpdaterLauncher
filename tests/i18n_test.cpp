#include "i18n.h"
#include "storage.h"
#include <filesystem>
#include <iostream>
#include <cstdlib>
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::cerr<<"Language test failed at "<<__LINE__<<": "<<#x<<"\n";std::abort();}}while(0)
int main(){
    const char* play[]={"Jouer","Play","Jugar","Gioca","Jogar"};
    const char* installed[]={"La version v1 est déjà installée.","Version v1 is already installed.","La versión v1 ya está instalada.","La versione v1 è già installata.","A versão v1 já está instalada."};
    std::filesystem::create_directories("build-tests/preferences");
    const std::string file="build-tests/preferences/language.txt",config="build-tests/preferences/config.ini";
    CHECK(writeText(config,"manifest_url=https://example.com/custom.ini\n"));
    for(int i=0;i<LANGUAGE_COUNT;i++){
        auto language=static_cast<Language>(i);setLanguage(language);
        CHECK(std::string(tr("Jouer"))==play[i]);
        CHECK(trf("La version {0} est déjà installée.",{"v1"})==installed[i]);
        CHECK(tr(std::string("untranslated technical value"))=="untranslated technical value");
        CHECK(saveLanguagePreference(file));CHECK(readText(file)==languageCode(language));
        setLanguage(Language::French);CHECK(loadLanguagePreference(file));CHECK(currentLanguage()==language);
        CHECK(readText(config)=="manifest_url=https://example.com/custom.ini\n");
        Language decoded=Language::French;CHECK(parseLanguageCode(std::string(languageCode(language))+"\r\n",decoded));CHECK(decoded==language);
        if(i!=0)CHECK(tr(std::string("Téléchargement incomplet."))!="Téléchargement incomplet.");
    }
    setLanguage(Language::English);
    for(auto bad:{"","de","pt-BR","EN","fr\nurl=bad"}){CHECK(writeText(file,bad));CHECK(!loadLanguagePreference(file));CHECK(currentLanguage()==Language::English);}
    CHECK(!saveLanguagePreference("build-tests/missing-folder/language.txt"));CHECK(currentLanguage()==Language::English);
    setLanguage(static_cast<Language>(99));CHECK(currentLanguage()==Language::French);
    std::cout<<checks<<" language and persistence checks passed.\n";
}
