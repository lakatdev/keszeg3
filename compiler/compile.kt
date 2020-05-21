import kotlinx.cinterop.*
import platform.posix.*

var instructions = ArrayList<UByte>()
var arguments = ArrayList<UByte>()

var numberInstructions = 0

private fun error(msg: String) {
    println(msg)
    exit(EXIT_FAILURE)
}

fun main(args: Array<String>) {
    if (args.size < 2) {
        error("Provide sufficient amount of arguments!")
    }

    var file = fopen(args[0], "r")
    fseek(file, 0, SEEK_END)

    var length = ftell(file).toULong()
    rewind(file)

    val buffer = ByteArray(length.toInt())
    buffer.usePinned { pinned ->
        fread(pinned.addressOf(0), length, 1, file)
    }
    fclose(file)

    var data = buffer.toKString().replace("\t"," ").replace("[ ]{2,}".toRegex(), " ").replace("\n ", "\n")
    var lines = ArrayList(data.replace("\r", "\n").split("\n"))

    lines = precompile(lines)
    lines = processFlags(lines)
    processCode(lines)

    save(args[1])
}

private fun convertCharacters(str: String): String {
    var retString = ""

    var index = 0
    while (index < str.length) {
        if (str[index] == '\\') {
            when (str[index + 1]) {
                'n' -> {
                    retString += '\n'
                }

                '\\' -> {
                    retString += '\\'
                }
            }
            index += 2
            continue
        }
        retString += str[index]
        index++
    }
    return retString
}

private fun addInstruction(type: UShort, argsize: UShort, args: UInt) {
    instructions.add((type.toUInt() and 0xFFu).toUByte())
    instructions.add(((type.toUInt() shr 8) and 0xFFu).toUByte())

    instructions.add((argsize.toUInt() and 0xFFu).toUByte())
    instructions.add(((argsize.toUInt() shr 8) and 0xFFu).toUByte())

    instructions.add((args.toUInt() and 0xFFu).toUByte())
    instructions.add(((args.toUInt() shr 8) and 0xFFu).toUByte())
    instructions.add(((args.toUInt() shr 16) and 0xFFu).toUByte())
    instructions.add(((args.toUInt() shr 24) and 0xFFu).toUByte())

    numberInstructions++
}

private fun addIntArguments(args: IntArray): UInt {
    val startLocation = arguments.size.toUInt()

    args.forEach {
        arguments.add((it.toUInt() and 0xFFu).toUByte())
        arguments.add(((it.toUInt() shr 8) and 0xFFu).toUByte())
        arguments.add(((it.toUInt() shr 16) and 0xFFu).toUByte())
        arguments.add(((it.toUInt() shr 24) and 0xFFu).toUByte())
    }

    return startLocation
}

private fun addBytesArgument(args: UByteArray): UInt {
    val startLocation = arguments.size.toUInt()

    args.forEach {
        arguments.add(it)
    }

    return startLocation
}

private fun String.isNumber(): Boolean {
    try {
        this.toInt()
    }
    catch (e: NumberFormatException) {
        return false
    }
    return true
}

private fun invalidToken(token: String): Boolean {
    val special: Regex = Regex("[a-zA-Z0-9]*")
    return !special.matches(token)
}

private fun checkTokensList(tokens: ArrayList<String>) {
    tokens.forEach {
        if (invalidToken(it)) {
            if (!(tokens[0] == "print" && tokens[1] == "string")) {
                error("ERROR: Invalid token")
            }
        }
    }
}

private fun save(path: String) {
    var numberInstructionsBytes = UByteArray(4)
    numberInstructionsBytes[0] = (numberInstructions and 0xFF).toUByte()
    numberInstructionsBytes[1] = ((numberInstructions shr 8) and 0xFF).toUByte()
    numberInstructionsBytes[2] = ((numberInstructions shr 16) and 0xFF).toUByte()
    numberInstructionsBytes[3] = ((numberInstructions shr 24) and 0xFF).toUByte()

    var numberVarsBytes = UByteArray(4)
    numberVarsBytes[0] = (Variables.getVarCount() and 0xFF).toUByte()
    numberVarsBytes[1] = ((Variables.getVarCount() shr 8) and 0xFF).toUByte()
    numberVarsBytes[2] = ((Variables.getVarCount() shr 16) and 0xFF).toUByte()
    numberVarsBytes[3] = ((Variables.getVarCount() shr 24) and 0xFF).toUByte()

    var numberArraysBytes = UByteArray(4)
    numberArraysBytes[0] = (Variables.getArrayCount() and 0xFF).toUByte()
    numberArraysBytes[1] = ((Variables.getArrayCount() shr 8) and 0xFF).toUByte()
    numberArraysBytes[2] = ((Variables.getArrayCount() shr 16) and 0xFF).toUByte()
    numberArraysBytes[3] = ((Variables.getArrayCount() shr 24) and 0xFF).toUByte()

    val save = fopen(path, "wb")
    val buffer = numberInstructionsBytes + numberVarsBytes + numberArraysBytes + (instructions + arguments).toUByteArray()
    val length = buffer.size.toULong()

    buffer.usePinned { pinned ->
        fwrite(pinned.addressOf(0), length, 1, save)
    }

    fclose(save)
}

private fun processFlags(lines: ArrayList<String>): ArrayList<String> {
    lines.forEachIndexed { index, it ->
        val tokens = ArrayList<String>(it.split(" "))
        checkTokensList(tokens)

        if (tokens[0] == "flag") {
            Variables.flags.add(Flag(tokens[1], index))
        }
    }

    for (i in 0 until Variables.flags.size) {
        lines.removeAt(Variables.getFlagPosition(Variables.flags[i].name) - i)
        Variables.setFlag(Variables.flags[i].name, Variables.flags[i].position - i)
    }

    return lines
}

private fun findEnd(lines: ArrayList<String>, position: Int): Int {
    var newifs = 1
    var seekPosition = position + 1

    while (newifs > 0) {
        seekPosition++
        var tokens = ArrayList<String>(lines[seekPosition].split(" "))

        if (tokens[0] == "if" || tokens[0] == "while") {
            newifs++
        }

        if (tokens[0] == "end") {
            newifs--
        }
    }
    return seekPosition
}

private fun precompile(lines: ArrayList<String>): ArrayList<String> {
    var ret = lines

    ret.forEachIndexed { index, it ->
        var tokens = ArrayList<String>(it.split(" "))
        checkTokensList(tokens)

        when (tokens[0]) {
            "while" -> {
                val generatedFlag = Variables.newFlag()

                tokens[0] = "if"
                ret[index] = tokens.joinToString(separator = " ")
                ret.add(index, "flag " + generatedFlag)

                ret.add(findEnd(ret, index), "jump " + generatedFlag)

                precompile(ret)
            }
        }
    }

    return ret
}

private fun processCode(lines: ArrayList<String>) {
    lines.forEach {
        val tokens = ArrayList<String>(it.split(" "))
        checkTokensList(tokens)

        handleLine(tokens)
    }
}

private fun handleLine(tokens: ArrayList<String>) {
    when (tokens[0]) {
        "set" -> {
            if (tokens[2].isNumber()) {
                addInstruction(Constants.SET_N, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), tokens[2].toInt())))
            }
            else {
                addInstruction(Constants.SET_V, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), Variables.getVarId(tokens[2]))))
            }
        }

        "if" -> {
            when (tokens[1]) {
                "more" -> {
                    if (tokens[2].isNumber()) {
                        if (tokens[3].isNumber()) {
                            addInstruction(Constants.IF_M_NN, 8u, addIntArguments(intArrayOf(tokens[2].toInt(), tokens[3].toInt())))
                        }
                        else {
                            addInstruction(Constants.IF_M_NV, 8u, addIntArguments(intArrayOf(tokens[2].toInt(), Variables.getVarId(tokens[3]))))
                        }
                    }
                    else {
                        if (tokens[3].isNumber()) {
                            addInstruction(Constants.IF_M_VN, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]), tokens[3].toInt())))
                        }
                        else {
                            addInstruction(Constants.IF_M_VV, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]), Variables.getVarId(tokens[3]))))
                        }
                    }
                }

                "less" -> {
                    if (tokens[2].isNumber()) {
                        if (tokens[3].isNumber()) {
                            addInstruction(Constants.IF_L_NN, 8u, addIntArguments(intArrayOf(tokens[2].toInt(), tokens[3].toInt())))
                        }
                        else {
                            addInstruction(Constants.IF_L_NV, 8u, addIntArguments(intArrayOf(tokens[2].toInt(), Variables.getVarId(tokens[3]))))
                        }
                    }
                    else {
                        if (tokens[3].isNumber()) {
                            addInstruction(Constants.IF_L_VN, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]), tokens[3].toInt())))
                        }
                        else {
                            addInstruction(Constants.IF_L_VV, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]), Variables.getVarId(tokens[3]))))
                        }
                    }
                }

                "equals" -> {
                    if (tokens[2].isNumber()) {
                        if (tokens[3].isNumber()) {
                            addInstruction(Constants.IF_E_NN, 8u, addIntArguments(intArrayOf(tokens[2].toInt(), tokens[3].toInt())))
                        }
                        else {
                            addInstruction(Constants.IF_E_NV, 8u, addIntArguments(intArrayOf(tokens[2].toInt(), Variables.getVarId(tokens[3]))))
                        }
                    }
                    else {
                        if (tokens[3].isNumber()) {
                            addInstruction(Constants.IF_E_VN, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]), tokens[3].toInt())))
                        }
                        else {
                            addInstruction(Constants.IF_E_VV, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]), Variables.getVarId(tokens[3]))))
                        }
                    }
                }

                "different" -> {
                    if (tokens[2].isNumber()) {
                        if (tokens[3].isNumber()) {
                            addInstruction(Constants.IF_D_NN, 8u, addIntArguments(intArrayOf(tokens[2].toInt(), tokens[3].toInt())))
                        }
                        else {
                            addInstruction(Constants.IF_D_NV, 8u, addIntArguments(intArrayOf(tokens[2].toInt(), Variables.getVarId(tokens[3]))))
                        }
                    }
                    else {
                        if (tokens[3].isNumber()) {
                            addInstruction(Constants.IF_D_VN, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]), tokens[3].toInt())))
                        }
                        else {
                            addInstruction(Constants.IF_D_VV, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]), Variables.getVarId(tokens[3]))))
                        }
                    }
                }

                "lessequals" -> {
                    if (tokens[2].isNumber()) {
                        if (tokens[3].isNumber()) {
                            addInstruction(Constants.IF_LE_NN, 8u, addIntArguments(intArrayOf(tokens[2].toInt(), tokens[3].toInt())))
                        }
                        else {
                            addInstruction(Constants.IF_LE_NV, 8u, addIntArguments(intArrayOf(tokens[2].toInt(), Variables.getVarId(tokens[3]))))
                        }
                    }
                    else {
                        if (tokens[3].isNumber()) {
                            addInstruction(Constants.IF_LE_VN, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]), tokens[3].toInt())))
                        }
                        else {
                            addInstruction(Constants.IF_LE_VV, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]), Variables.getVarId(tokens[3]))))
                        }
                    }
                }

                "moreequals" -> {
                    if (tokens[2].isNumber()) {
                        if (tokens[3].isNumber()) {
                            addInstruction(Constants.IF_ME_NN, 8u, addIntArguments(intArrayOf(tokens[2].toInt(), tokens[3].toInt())))
                        }
                        else {
                            addInstruction(Constants.IF_ME_NV, 8u, addIntArguments(intArrayOf(tokens[2].toInt(), Variables.getVarId(tokens[3]))))
                        }
                    }
                    else {
                        if (tokens[3].isNumber()) {
                            addInstruction(Constants.IF_ME_VN, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]), tokens[3].toInt())))
                        }
                        else {
                            addInstruction(Constants.IF_ME_VV, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]), Variables.getVarId(tokens[3]))))
                        }
                    }
                }
            }
        }

        "add" -> {
            if (tokens[2].isNumber()) {
                if (tokens[3].isNumber()) {
                    addInstruction(Constants.ADD_NN, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), tokens[2].toInt(), tokens[3].toInt())))
                }
                else {
                    addInstruction(Constants.ADD_NV, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), tokens[2].toInt(), Variables.getVarId(tokens[3]))))
                }
            }
            else {
                if (tokens[3].isNumber()) {
                    addInstruction(Constants.ADD_VN, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), Variables.getVarId(tokens[2]), tokens[3].toInt())))
                }
                else {
                    addInstruction(Constants.ADD_VV, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), Variables.getVarId(tokens[2]), Variables.getVarId(tokens[3]))))
                }
            }
        }

        "array" -> {
            addInstruction(Constants.ARRAY, 8u, addIntArguments(intArrayOf(Variables.getArrayId(tokens[1]), tokens[2].toInt())))
        }

        "sub" -> {
            if (tokens[2].isNumber()) {
                if (tokens[3].isNumber()) {
                    addInstruction(Constants.SUB_NN, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), tokens[2].toInt(), tokens[3].toInt())))
                }
                else {
                    addInstruction(Constants.SUB_NV, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), tokens[2].toInt(), Variables.getVarId(tokens[3]))))
                }
            }
            else {
                if (tokens[3].isNumber()) {
                    addInstruction(Constants.SUB_VN, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), Variables.getVarId(tokens[2]), tokens[3].toInt())))
                }
                else {
                    addInstruction(Constants.SUB_VV, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), Variables.getVarId(tokens[2]), Variables.getVarId(tokens[3]))))
                }
            }
        }

        "mul" -> {
            if (tokens[2].isNumber()) {
                if (tokens[3].isNumber()) {
                    addInstruction(Constants.MUL_NN, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), tokens[2].toInt(), tokens[3].toInt())))
                }
                else {
                    addInstruction(Constants.MUL_NV, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), tokens[2].toInt(), Variables.getVarId(tokens[3]))))
                }
            }
            else {
                if (tokens[3].isNumber()) {
                    addInstruction(Constants.MUL_VN, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), Variables.getVarId(tokens[2]), tokens[3].toInt())))
                }
                else {
                    addInstruction(Constants.MUL_VV, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), Variables.getVarId(tokens[2]), Variables.getVarId(tokens[3]))))
                }
            }
        }

        "div" -> {
            if (tokens[2].isNumber()) {
                if (tokens[3].isNumber()) {
                    addInstruction(Constants.DIV_NN, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), tokens[2].toInt(), tokens[3].toInt())))
                }
                else {
                    addInstruction(Constants.DIV_NV, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), tokens[2].toInt(), Variables.getVarId(tokens[3]))))
                }
            }
            else {
                if (tokens[3].isNumber()) {
                    addInstruction(Constants.DIV_VN, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), Variables.getVarId(tokens[2]), tokens[3].toInt())))
                }
                else {
                    addInstruction(Constants.DIV_VV, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), Variables.getVarId(tokens[2]), Variables.getVarId(tokens[3]))))
                }
            }
        }

        "print" -> {
            when (tokens[1]) {
                "num" -> {
                    if (tokens[2].isNumber()) {
                        addInstruction(Constants.PRINT_NUM_N, 4u, addIntArguments(intArrayOf(tokens[2].toInt())))
                    }
                    else {
                        addInstruction(Constants.PRINT_NUM_V, 4u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]))))
                    }
                }

                "ascii" -> {
                    if (tokens[2].isNumber()) {
                        val byteArray = UByteArray(1)
                        byteArray[0] = tokens[2].toUByte()
                        addInstruction(Constants.PRINT_ASCII_N, 4u, addBytesArgument(byteArray))
                    }
                    else {
                        addInstruction(Constants.PRINT_ASCII_V, 4u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]))))
                    }
                }

                "string" -> {
                    var strToPrint = tokens[2]
                    for (i in 3 until tokens.size) {
                        strToPrint += (" " + tokens[i])
                    }
                    val stringInBytes = convertCharacters(strToPrint).encodeToByteArray().toUByteArray()
                    addInstruction(Constants.PRINT_STRING, stringInBytes.size.toUShort(), addBytesArgument(stringInBytes))
                }
            }
        }

        "input" -> {
            when (tokens[1]) {
                "num" -> {
                    addInstruction(Constants.INPUT_NUM, 4u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]))))
                }

                "ascii" -> {
                    addInstruction(Constants.INPUT_ASCII, 4u, addIntArguments(intArrayOf(Variables.getVarId(tokens[2]))))
                }
            }
        }

        "mod" -> {
            if (tokens[2].isNumber()) {
                if (tokens[3].isNumber()) {
                    addInstruction(Constants.MOD_NN, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), tokens[2].toInt(), tokens[3].toInt())))
                }
                else {
                    addInstruction(Constants.MOD_NV, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), tokens[2].toInt(), Variables.getVarId(tokens[3]))))
                }
            }
            else {
                if (tokens[3].isNumber()) {
                    addInstruction(Constants.MOD_VN, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), Variables.getVarId(tokens[2]), tokens[3].toInt())))
                }
                else {
                    addInstruction(Constants.MOD_VV, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), Variables.getVarId(tokens[2]), Variables.getVarId(tokens[3]))))
                }
            }
        }

        "end" -> {
            addInstruction(Constants.END, 0u, 0u)
        }

        "arrset" -> {
            if (tokens[2].isNumber()) {
                if (tokens[3].isNumber()) {
                    addInstruction(Constants.ARRSET_NN, 12u, addIntArguments(intArrayOf(Variables.getArrayId(tokens[1]), tokens[2].toInt(), tokens[3].toInt())))
                }
                else {
                    addInstruction(Constants.ARRSET_NV, 12u, addIntArguments(intArrayOf(Variables.getArrayId(tokens[1]), tokens[2].toInt(), Variables.getVarId(tokens[3]))))
                }
            }
            else {
                if (tokens[3].isNumber()) {
                    addInstruction(Constants.ARRSET_VN, 12u, addIntArguments(intArrayOf(Variables.getArrayId(tokens[1]), Variables.getVarId(tokens[2]), tokens[3].toInt())))
                }
                else {
                    addInstruction(Constants.ARRSET_VV, 12u, addIntArguments(intArrayOf(Variables.getArrayId(tokens[1]), Variables.getVarId(tokens[2]), Variables.getVarId(tokens[3]))))
                }
            }
        }

        "arrget" -> {
            if (tokens[3].isNumber()) {
                addInstruction(Constants.ARRGET_N, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), Variables.getArrayId(tokens[2]), tokens[3].toInt())))
            }
            else {
                addInstruction(Constants.ARRGET_V, 12u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), Variables.getArrayId(tokens[2]), Variables.getVarId(tokens[3]))))
            }
        }

        "inc" -> {
            if (tokens.size >= 3) {
                if (tokens[2].isNumber()) {
                    addInstruction(Constants.INC_N, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), tokens[2].toInt())))
                }
                else {
                    addInstruction(Constants.INC_V, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), Variables.getVarId(tokens[2]))))
                }
            }
            else {
                addInstruction(Constants.INC_N, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), 1)))
            }
        }

        "dec" -> {
            if (tokens.size >= 3) {
                if (tokens[2].isNumber()) {
                    addInstruction(Constants.DEC_N, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), tokens[2].toInt())))
                }
                else {
                    addInstruction(Constants.DEC_V, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), Variables.getVarId(tokens[2]))))
                }
            }
            else {
                addInstruction(Constants.DEC_N, 8u, addIntArguments(intArrayOf(Variables.getVarId(tokens[1]), 1)))
            }
        }

        "jump" -> {
            addInstruction(Constants.JUMP, 4u, addIntArguments(intArrayOf(Variables.getFlagPosition(tokens[1]))))
        }

        "free" -> {
            addInstruction(Constants.FREE_ARRAY, 4u, addIntArguments(intArrayOf(Variables.getArrayId(tokens[1]))))
        }

        "exec" -> {
            var strToPrint = tokens[1]
            for (i in 2 until tokens.size) {
                strToPrint += (" " + tokens[i])
            }
            val stringInBytes = convertCharacters(strToPrint).encodeToByteArray().toUByteArray()
            addInstruction(Constants.EXEC, stringInBytes.size.toUShort(), addBytesArgument(stringInBytes))
        }
    }
}