#include "instTable.h"

static char *loadInstTable() {
    FILE *fp;
    long lSize;
    fp = fopen("8086_inst_table.txt", "rb");
    assert(fp && "Failed to open file");

    fseek(fp, 0L, SEEK_END);
    lSize = ftell(fp);
    rewind(fp);

    char *instTableStr = (char *) calloc(1, lSize + 1);
    assert(instTableStr && "Failed to allocate buffer");

    int res = fread(instTableStr, lSize, 1, fp);
    assert(res == 1);
    fclose(fp);

    return instTableStr;
}

static InstrPart decodeInstrPart(char *str) {
    InstrPart ip{};
    if (str[0] == '1' || str[0] == '0') { // Literal Bits
        ip.type = IP_BITS;
        ip.bits.data = strtol(str, NULL, 2);
        ip.bits.size = strlen(str);
    } else if (strcmp(str, "D") == 0) {
        ip.type = IP_D;
    } else if (strcmp(str, "W") == 0) {
        ip.type = IP_W;
    } else if (strcmp(str, "S") == 0) {
        ip.type = IP_S;
    } else if (strcmp(str, "V") == 0) {
        ip.type = IP_V;
    } else if (strcmp(str, "Z") == 0) {
        ip.type = IP_Z;
    } else if (strcmp(str, "MOD") == 0) {
        ip.type = IP_MOD;
    } else if (strcmp(str, "REG") == 0) {
        ip.type = IP_REG;
    } else if (strcmp(str, "RM") == 0) {
        ip.type = IP_RM;
    } else if (strcmp(str, "DATA") == 0) {
        ip.type = IP_DATA;
    } else if (strcmp(str, "DATA_IF_W") == 0) {
        ip.type = IP_DATA_IF_W;
    } else if (strcmp(str, "ADDR") == 0) {
        ip.type = IP_ADDR;
    } else if (strcmp(str, "DISP") == 0) {
        ip.type = IP_DISP;
    } else if (strcmp(str, "SR") == 0) {
        ip.type = IP_SR;
    } else if (strncmp(str, "IMP_D(", 6) == 0) {
        ip.type = IP_IMP_D;
        ip.impD = strtol(str + 6, NULL, 2);
    } else if (strncmp(str, "IMP_W(", 6) == 0) {
        ip.type = IP_IMP_W;
        ip.impW = strtol(str + 6, NULL, 2);
    } else if (strncmp(str, "IMP_REG(", 8) == 0) {
        ip.type = IP_IMP_REG;
        ip.impReg = strtol(str + 8, NULL, 2);
    } else if (strncmp(str, "IMP_MOD(", 8) == 0) {
        ip.type = IP_IMP_MOD;
        ip.impMod = strtol(str + 8, NULL, 2);
    } else if (strncmp(str, "IMP_RM(", 7) == 0) {
        ip.type = IP_IMP_RM;
        ip.impRm = strtol(str + 7, NULL, 2);
    } else if (strcmp(str, "F_RM_REG_WIDE") == 0) {
        ip.type = IP_F_RM_REG_WIDE;
    } else {
        fprintf(stderr, "Failed to parse instruction part: %s\n", str);
        exit(1);
    }

    return ip;
}

std::vector<InstrDef> getInstTable() {
    char *instTableStr = loadInstTable();

    char *line, *lineTokCtx;
    line = strtok_s(instTableStr, "\r\n", &lineTokCtx);
    std::vector<InstrDef> defs;

    do {
        InstrDef def{};
        char *token = strtok(line, " ");
        def.op = decodeOpStr(token);

        token = strtok(NULL, " ");
        if (token == NULL) {
            fprintf(stderr, "Instruction declared without data!");
            exit(1);
        }

        std::vector<InstrPart> parts;
        do {
            parts.push_back(decodeInstrPart(token));
        } while ((token = strtok(NULL, " ")) != NULL);
        def.parts = parts;

        defs.push_back(def);
    } while ((line = strtok_s(lineTokCtx, "\r\n", &lineTokCtx)) != NULL);

    free(instTableStr);
    return defs;
}

