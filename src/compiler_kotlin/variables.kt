class Flag(var name: String, var position: Int)

@ThreadLocal
object Variables {

    private var arrs = ArrayList<String>()
    private var vars = ArrayList<String>()

    private val charPool : List<Char> = ('a'..'z') + ('A'..'Z') + ('0'..'9')
    private fun randomString(length: Int): String{
        return (1..length)
            .map { _ -> kotlin.random.Random.nextInt(0, charPool.size) }
            .map(charPool::get)
            .joinToString("")
    }

    var flags = ArrayList<Flag>()

    fun getVarId(name: String): Int {
        if (!vars.contains(name)) {
            vars.add(name)
        }
        return vars.indexOf(name)
    }

    fun getVarCount(): Int {
        return vars.size
    }

    fun getArrayId(name: String): Int {
        if (!arrs.contains(name)) {
            arrs.add(name)
        }
        return arrs.indexOf(name)
    }

    fun getArrayCount(): Int {
        return arrs.size
    }

    fun newFlag(): String {
        var proposedName = randomString(20)

        flags.forEach {
            if (it.name == proposedName) {
                return newFlag()
            }
        }

        return proposedName
    }

    fun setFlag(name: String, position: Int) {
        flags.forEach {
            if (name == it.name) {
                it.position = position
            }
        }
    }

    fun getFlagPosition(name: String): Int {
        flags.forEach {
            if (name == it.name) {
                return it.position
            }
        }
        return 0
    }
}