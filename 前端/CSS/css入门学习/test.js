const formatNumber = n => {
    n = n.toString()
    return n[1] ? n : '0' + n
}

var y = formatNumber(11)
console.log(y)