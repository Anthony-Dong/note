window.onload = function () {
    var oReq = new XMLHttpRequest();
    oReq.addEventListener("load", transferComplete);
    oReq.open(
        "GET",
        "http://47.94.234.232:9000/blog/increase/1/825937477500928"
    );
    oReq.send();

    function transferComplete(evt) {
        console.log("objects :", evt.target.response);
        document.getElementById("capp").innerHTML = JSON.parse(
            evt.target.response
        ).value;
    }
};