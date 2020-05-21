@ThreadLocal
object Constants {

    val SET_N: UShort = 0x2u
    val SET_V: UShort = 0x3u

    val IF_M_NN: UShort = 0x4u
    val IF_M_NV: UShort = 0x5u
    val IF_M_VN: UShort = 0x6u
    val IF_M_VV: UShort = 0x7u

    val IF_L_NN: UShort = 0x8u
    val IF_L_NV: UShort = 0x9u
    val IF_L_VN: UShort = 0xAu
    val IF_L_VV: UShort = 0xBu

    val IF_E_NN: UShort = 0xCu
    val IF_E_NV: UShort = 0xDu
    val IF_E_VN: UShort = 0xEu
    val IF_E_VV: UShort = 0xFu

    val IF_D_NN: UShort = 0x10u
    val IF_D_NV: UShort = 0x11u
    val IF_D_VN: UShort = 0x12u
    val IF_D_VV: UShort = 0x13u

    val IF_LE_NN: UShort = 0x14u
    val IF_LE_NV: UShort = 0x15u
    val IF_LE_VN: UShort = 0x16u
    val IF_LE_VV: UShort = 0x17u

    val IF_ME_NN: UShort = 0x18u
    val IF_ME_NV: UShort = 0x19u
    val IF_ME_VN: UShort = 0x1Au
    val IF_ME_VV: UShort = 0x1Bu

    val ADD_NN: UShort = 0x1Cu
    val ADD_NV: UShort = 0x1Du
    val ADD_VN: UShort = 0x1Eu
    val ADD_VV: UShort = 0x1Fu

    val ARRAY: UShort = 0x20u

    val SUB_NN: UShort = 0x21u
    val SUB_NV: UShort = 0x22u
    val SUB_VN: UShort = 0x23u
    val SUB_VV: UShort = 0x24u

    val MUL_NN: UShort = 0x25u
    val MUL_NV: UShort = 0x26u
    val MUL_VN: UShort = 0x27u
    val MUL_VV: UShort = 0x28u

    val DIV_NN: UShort = 0x29u
    val DIV_NV: UShort = 0x2Au
    val DIV_VN: UShort = 0x2Bu
    val DIV_VV: UShort = 0x2Cu

    val PRINT_NUM_N: UShort = 0x2Du
    val PRINT_NUM_V: UShort = 0x2Eu

    val PRINT_ASCII_N: UShort = 0x2Fu
    val PRINT_ASCII_V: UShort = 0x30u

    val PRINT_STRING: UShort = 0x31u

    val INPUT_NUM: UShort = 0x32u

    val INPUT_ASCII: UShort = 0x33u

    val MOD_NN: UShort = 0x34u
    val MOD_NV: UShort = 0x35u
    val MOD_VN: UShort = 0x36u
    val MOD_VV: UShort = 0x37u

    val END: UShort = 0x38u

    val ARRSET_NN: UShort = 0x39u
    val ARRSET_NV: UShort = 0x3Au
    val ARRSET_VN: UShort = 0x3Bu
    val ARRSET_VV: UShort = 0x3Cu

    val ARRGET_N: UShort = 0x3Du
    val ARRGET_V: UShort = 0x3Eu

    val INC_N: UShort = 0x3Fu
    val INC_V: UShort = 0x40u

    val DEC_N: UShort = 0x41u
    val DEC_V: UShort = 0x42u

    val JUMP: UShort = 0x1u

    val FREE_ARRAY: UShort = 0x43u

    val EXEC: UShort = 0x44u
}