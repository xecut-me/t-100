#ifndef __CONVERT_H__
#define __CONVERT_H__


#define BAUD_NULL    0x00      // Null symbol
#define BAUD_E_3     0x01      // E or 3
#define BAUD_LF      0x02      // line feed
#define BAUD_A_MINUS 0x03      // A or -
#define BAUD_SPACE   0x04      // ' '
#define BAUD_S_APO   0x05      // S or '
#define BAUD_I_8     0x06      // I or 8
#define BAUD_U_7     0x07      // U or 7
#define BAUD_CR      0x08      // carriage return
#define BAUD_D_CURR  0x09      // D or $ (or WRU request)
#define BAUD_R_4     0x0a      // R or 4
#define BAUD_J_BELL  0x0b      // J or bell
#define BAUD_N_COMA  0x0c      // N or ,
#define BAUD_F_EXCL  0x0d      // F or !
#define BAUD_C_COLO  0x0e      // C or :
#define BAUD_K_BR_L  0x0f      // K or (
#define BAUD_T_5     0x10      // T or 5
#define BAUD_Z_PLUS  0x11      // Z or +
#define BAUD_L_BR_R  0x12      // L or )
#define BAUD_W_2     0x13      // W or 2
#define BAUD_H_HASH  0x14      // H or #
#define BAUD_Y_6     0x15      // Y or 6
#define BAUD_P_0     0x16      // P or 0
#define BAUD_Q_1     0x17      // Q or 1
#define BAUD_O_9     0x18      // O or 9
#define BAUD_B_QUES  0x19      // B or ?
#define BAUD_G_AMP   0x1a      // G or &
#define BAUD_FIGS    0x1b      // switch to figures
#define BAUD_M_STOP  0x1c      // M or .
#define BAUD_X_SLAS  0x1d      // X or /
#define BAUD_V_EQUA  0x1e      // V or =
#define BAUD_LTRS    0x1f      // switch to letters
#define FIGS    0x1b
#define LTRS    0x1f


#define SPECIAL     0b10000000
#define CANFIG      0b01000000
#define CANLTR      0b00100000

const uint8_t ascii2baudot[128] PROGMEM = 
{ 
    CANFIG|CANLTR|0, //  , hex: 0x00
    0, //  , hex: 0x01
    0, //  , hex: 0x02
    0, //  , hex: 0x03
    CANFIG|BAUD_H_HASH, //  , hex: 0x04 // motor stop (#)
    0, //  , hex: 0x05
    0, //  , hex: 0x06
    CANFIG|BAUD_J_BELL, //  , hex: 0x07
    0, //  , hex: 0x08
    0, //  , hex: 0x09
    CANFIG|CANLTR|BAUD_LF, //  , hex: 0x0a
    SPECIAL|1, //  , hex: 0x0b    //vertical tab, replace with four spaces
    SPECIAL|2, //  , hex: 0x0c    // form feed, replace with two line feeds
    CANFIG|CANLTR|BAUD_CR, //  , hex: 0x0d
    0, //  , hex: 0x0e
    0, //  , hex: 0x0f
    0, //  , hex: 0x10
    0, //  , hex: 0x11
    0, //  , hex: 0x12
    0, //  , hex: 0x13
    0, //  , hex: 0x14
    0, //  , hex: 0x15
    0, //  , hex: 0x16
    0, //  , hex: 0x17
    SPECIAL|3, //  , hex: 0x18     // ESC, replace with (ESC) ?
    0, //  , hex: 0x19
    0, //  , hex: 0x1a
    0, //  , hex: 0x1b
    0, //  , hex: 0x1c
    0, //  , hex: 0x1d
    0, //  , hex: 0x1e
    0, //  , hex: 0x1f
    CANFIG|CANLTR|BAUD_SPACE,  // ' ' hex: 0x20
    CANFIG|BAUD_F_EXCL, // ! hex: 0x21
    CANFIG|BAUD_S_APO,  // " hex: 0x22
    CANFIG|BAUD_H_HASH, // # hex: 0x23
    CANFIG|BAUD_D_CURR, // $ hex: 0x24
    CANFIG|BAUD_X_SLAS, // % hex: 0x25
    CANFIG|BAUD_G_AMP, // & hex: 0x26
    CANFIG|BAUD_S_APO, // ' hex: 0x27
    CANFIG|BAUD_K_BR_L, // ( hex: 0x28
    CANFIG|BAUD_L_BR_R, // ) hex: 0x29
    CANLTR|BAUD_X_SLAS, // * hex: 0x2a
    CANFIG|BAUD_Z_PLUS, // + hex: 0x2b
    CANFIG|BAUD_N_COMA, // , hex: 0x2c
    CANFIG|BAUD_A_MINUS, // - hex: 0x2d
    CANFIG|BAUD_M_STOP, // . hex: 0x2e
    CANFIG|BAUD_X_SLAS, // / hex: 0x2f
    CANFIG|BAUD_P_0, // 0 hex: 0x30
    CANFIG|BAUD_Q_1, // 1 hex: 0x31
    CANFIG|BAUD_W_2, // 2 hex: 0x32
    CANFIG|BAUD_E_3, // 3 hex: 0x33
    CANFIG|BAUD_R_4, // 4 hex: 0x34
    CANFIG|BAUD_T_5, // 5 hex: 0x35
    CANFIG|BAUD_Y_6, // 6 hex: 0x36
    CANFIG|BAUD_U_7, // 7 hex: 0x37
    CANFIG|BAUD_I_8, // 8 hex: 0x38
    CANFIG|BAUD_O_9, // 9 hex: 0x39
    CANFIG|BAUD_C_COLO, // : hex: 0x3a
    CANFIG|BAUD_C_COLO, // ; hex: 0x3b
    CANFIG|BAUD_K_BR_L, // < hex: 0x3c
    CANFIG|BAUD_V_EQUA, // = hex: 0x3d
    CANFIG|BAUD_L_BR_R, // > hex: 0x3e
    CANFIG|BAUD_B_QUES, // ? hex: 0x3f
    CANFIG|BAUD_H_HASH, // @ hex: 0x40
    CANLTR|BAUD_A_MINUS, // A hex: 0x41
    CANLTR|BAUD_B_QUES, // B hex: 0x42
    CANLTR|BAUD_C_COLO, // C hex: 0x43
    CANLTR|BAUD_D_CURR, // D hex: 0x44
    CANLTR|BAUD_E_3, // E hex: 0x45
    CANLTR|BAUD_F_EXCL, // F hex: 0x46
    CANLTR|BAUD_G_AMP, // G hex: 0x47
    CANLTR|BAUD_H_HASH, // H hex: 0x48
    CANLTR|BAUD_I_8, // I hex: 0x49
    CANLTR|BAUD_J_BELL, // J hex: 0x4a
    CANLTR|BAUD_K_BR_L, // K hex: 0x4b
    CANLTR|BAUD_L_BR_R, // L hex: 0x4c
    CANLTR|BAUD_M_STOP, // M hex: 0x4d
    CANLTR|BAUD_N_COMA, // N hex: 0x4e
    CANLTR|BAUD_O_9, // O hex: 0x4f
    CANLTR|BAUD_P_0, // P hex: 0x50
    CANLTR|BAUD_Q_1, // Q hex: 0x51
    CANLTR|BAUD_R_4, // R hex: 0x52
    CANLTR|BAUD_S_APO, // S hex: 0x53
    CANLTR|BAUD_T_5, // T hex: 0x54
    CANLTR|BAUD_U_7, // U hex: 0x55
    CANLTR|BAUD_V_EQUA, // V hex: 0x56
    CANLTR|BAUD_W_2, // W hex: 0x57
    CANLTR|BAUD_X_SLAS, // X hex: 0x58
    CANLTR|BAUD_Y_6, // Y hex: 0x59
    CANLTR|BAUD_Z_PLUS, // Z hex: 0x5a
    CANFIG|BAUD_K_BR_L, // [ hex: 0x5b
    CANFIG|BAUD_X_SLAS, // \ hex: 0x5c
    CANFIG|BAUD_L_BR_R, // ] hex: 0x5d
    CANFIG|BAUD_S_APO, // ^ hex: 0x5e
    CANFIG|BAUD_A_MINUS, // _ hex: 0x5f
    CANFIG|BAUD_S_APO, // ` hex: 0x60
    CANLTR|BAUD_A_MINUS, // a hex: 0x61
    CANLTR|BAUD_B_QUES, // b hex: 0x62
    CANLTR|BAUD_C_COLO, // c hex: 0x63
    CANLTR|BAUD_D_CURR, // d hex: 0x64
    CANLTR|BAUD_E_3, // e hex: 0x65
    CANLTR|BAUD_F_EXCL, // f hex: 0x66
    CANLTR|BAUD_G_AMP, // g hex: 0x67
    CANLTR|BAUD_H_HASH, // h hex: 0x68
    CANLTR|BAUD_I_8, // i hex: 0x69
    CANLTR|BAUD_J_BELL, // j hex: 0x6a
    CANLTR|BAUD_K_BR_L, // k hex: 0x6b
    CANLTR|BAUD_L_BR_R, // l hex: 0x6c
    CANLTR|BAUD_M_STOP, // m hex: 0x6d
    CANLTR|BAUD_N_COMA, // n hex: 0x6e
    CANLTR|BAUD_O_9, // o hex: 0x6f
    CANLTR|BAUD_P_0, // p hex: 0x70
    CANLTR|BAUD_Q_1, // q hex: 0x71
    CANLTR|BAUD_R_4, // r hex: 0x72
    CANLTR|BAUD_S_APO, // s hex: 0x73
    CANLTR|BAUD_T_5, // t hex: 0x74
    CANLTR|BAUD_U_7, // u hex: 0x75
    CANLTR|BAUD_V_EQUA, // v hex: 0x76
    CANLTR|BAUD_W_2, // w hex: 0x77
    CANLTR|BAUD_X_SLAS, // x hex: 0x78
    CANLTR|BAUD_Y_6, // y hex: 0x79
    CANLTR|BAUD_Z_PLUS, // z hex: 0x7a
    CANFIG|BAUD_K_BR_L, // { hex: 0x7b
    CANLTR|BAUD_L_BR_R, // | hex: 0x7c
    CANFIG|BAUD_L_BR_R, // } hex: 0x7d
    CANFIG|BAUD_A_MINUS, // ~ hex: 0x7e
    0, //   hex: 0x7f
};


// lower part - LTRS, upper part - FIGS
const uint8_t baudot2ascii[64] PROGMEM = 
{
       0, //BAUD_NULL    0x00      // Null symbol
     'E', //BAUD_E_3     0x01      // E or 3
    '\n', //BAUD_LF      0x02      // line feed
     'A', //BAUD_A_MINUS 0x03      // A or -
     ' ', //BAUD_SPACE   0x04      // ' '
     'S', //BAUD_S_APO   0x05      // S or '
     'I', //BAUD_I_8     0x06      // I or 8
     'U', //BAUD_U_7     0x07      // U or 7
    '\r', //BAUD_CR      0x08      // carriage return
     'D', //BAUD_D_CURR  0x09      // D or $ (or WRU request)
     'R', //BAUD_R_4     0x0a      // R or 4
     'J', //BAUD_J_BELL  0x0b      // J or bell
     'N', //BAUD_N_COMA  0x0c      // N or ,
     'F', //BAUD_F_EXCL  0x0d      // F or !
     'C', //BAUD_C_COLO  0x0e      // C or :
     'K', //BAUD_K_BR_L  0x0f      // K or (
     'T', //BAUD_T_5     0x10      // T or 5
     'Z', //BAUD_Z_PLUS  0x11      // Z or +
     'L', //BAUD_L_BR_R  0x12      // L or )
     'W', //BAUD_W_2     0x13      // W or 2
     'H', //BAUD_H_HASH  0x14      // H or #
     'Y', //BAUD_Y_6     0x15      // Y or 6
     'P', //BAUD_P_0     0x16      // P or 0
     'Q', //BAUD_Q_1     0x17      // Q or 1
     'O', //BAUD_O_9     0x18      // O or 9
     'B', //BAUD_B_QUES  0x19      // B or ?
     'G', //BAUD_G_AMP   0x1a      // G or &
       0, //BAUD_FIGS    0x1b      // switch to figures
     'M', //BAUD_M_STOP  0x1c      // M or .
     'X', //BAUD_X_SLAS  0x1d      // X or /
     'V', //BAUD_V_EQUA  0x1e      // V or =
       0, //BAUD_LTRS    0x1f      // switch to letters

       0, //BAUD_NULL    0x00      // Null symbol
     '3', //BAUD_E_3     0x01      // E or 3
    '\n', //BAUD_LF      0x02      // line feed
     '-', //BAUD_A_MINUS 0x03      // A or -
     ' ', //BAUD_SPACE   0x04      // ' '
    '\'', //BAUD_S_APO   0x05      // S or '
     '8', //BAUD_I_8     0x06      // I or 8
     '7', //BAUD_U_7     0x07      // U or 7
    '\r', //BAUD_CR      0x08      // carriage return
     '$', //BAUD_D_CURR  0x09      // D or $ (or WRU request)
     '4', //BAUD_R_4     0x0a      // R or 4
    '\a', //BAUD_J_BELL  0x0b      // J or bell
     ',', //BAUD_N_COMA  0x0c      // N or ,
     '!', //BAUD_F_EXCL  0x0d      // F or !
     ':', //BAUD_C_COLO  0x0e      // C or :
     '(', //BAUD_K_BR_L  0x0f      // K or (
     '5', //BAUD_T_5     0x10      // T or 5
     '+', //BAUD_Z_PLUS  0x11      // Z or +
     ')', //BAUD_L_BR_R  0x12      // L or )
     '2', //BAUD_W_2     0x13      // W or 2
     '#', //BAUD_H_HASH  0x14      // H or #
     '6', //BAUD_Y_6     0x15      // Y or 6
     '0', //BAUD_P_0     0x16      // P or 0
     '1', //BAUD_Q_1     0x17      // Q or 1
     '9', //BAUD_O_9     0x18      // O or 9
     '?', //BAUD_B_QUES  0x19      // B or ?
     '&', //BAUD_G_AMP   0x1a      // G or &
       0, //BAUD_FIGS    0x1b      // switch to figures
     '.', //BAUD_M_STOP  0x1c      // M or .
     '/', //BAUD_X_SLAS  0x1d      // X or /
     '=', //BAUD_V_EQUA  0x1e      // V or =
       0, //BAUD_LTRS    0x1f      // switch to letters
};

#endif //__CONVERT_H__
