const fun1 = function (obj) {
    var ok = `{"state":"ok"}`
    obj["success"](ok)
}

fun1({
    "success": function (res) {
        console.log('res :', res);
        console.log('this :', this);
    }
})