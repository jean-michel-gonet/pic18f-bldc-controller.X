#include "test.h"
#include "file.h"

/**
 * Si il y a de la place dans la file, enfile un caractère.
 * @param c Le caractère.
 */
void fileEnfile(File *file, char c) {
    file->fileVide = 0;
    if (!file->filePleine) {
        file->file[file->fileEntree++] = c;
        if (file->fileEntree >= FILE_TAILLE) {
            file->fileEntree = 0;
        }
        if (file->fileEntree == file->fileSortie) {
            file->filePleine = 255;
        }
    }
}

/**
 * Si la file n'est pas vide, défile un caractère.
 * @return Le caractère défilé, ou 0 si la file est vide.
 */
char fileDefile(File *file) {
    char c;
    file->filePleine = 0;
    if (!file->fileVide) {
        c = file->file[file->fileSortie++];
        if (file->fileSortie >= FILE_TAILLE) {
            file->fileSortie = 0;
        }
        if (file->fileSortie == file->fileEntree) {
            file->fileVide = 255;
        }
        return c;
    }
    return 0;
}

/**
 * Indique si la file est vide.
 */
char fileEstVide(File *file) {
    return file->fileVide;
}

/**
 * Indique si la file est pleine.
 */
char fileEstPleine(File *file) {
    return file->filePleine;
}

/**
 * Vide et réinitialise la file.
 */
void fileReinitialise(File *file) {
    file->fileEntree = 0;
    file->fileSortie = 0;
    file->fileVide = 255;
    file->filePleine = 0;
    
}

#ifdef TEST
void testEnfileEtDefile() {
    File file;
    fileReinitialise(&file);
    
    verifieEgalite("FIL01", fileEstVide(&file), 255);    
    verifieEgalite("FIL02", fileDefile(&file), 0);
    verifieEgalite("FIL03", fileDefile(&file), 0);

    fileEnfile(&file, 10);
    fileEnfile(&file, 20);

    verifieEgalite("FIL04", fileEstVide(&file), 0);
    verifieEgalite("FIL05", fileDefile(&file), 10);
    verifieEgalite("FIL06", fileDefile(&file), 20);
    verifieEgalite("FIL07", fileEstVide(&file), 255);
    verifieEgalite("FIL08", fileDefile(&file), 0);
}

void testEnfileEtDefileBeaucoupDeCaracteres() {
    File file;
    int n = 0;
    char c = 0;
    
    fileReinitialise(&file);

    for (n = 0; n < FILE_TAILLE * 4; n++) {
        fileEnfile(&file, c);
        if (verifieEgalite("FBC001", fileDefile(&file), c)) {
            return;
        }
        c++;
    }
}

void testDebordePuisRecupereLesCaracteres() {
    File file;
    char c = 1;
    
    fileReinitialise(&file);
    while(!fileEstPleine(&file)) {
        fileEnfile(&file, c++);
    }

    verifieEgalite("FDB001", fileDefile(&file), 1);
    verifieEgalite("FDB002", fileDefile(&file), 2);
    
    while(!fileEstVide(&file)) {
        c = fileDefile(&file);
    }
    fileEnfile(&file, 1);      // Ces caractères sont ignorés...
    fileEnfile(&file, 1);      // ... car la file est pleine.

    verifieEgalite("FDB003", c, FILE_TAILLE);
}

int test_file() {
    testEnfileEtDefile();
    testEnfileEtDefileBeaucoupDeCaracteres();
    testDebordePuisRecupereLesCaracteres();
}
#endif
