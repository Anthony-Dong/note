//隔行变色及鼠标经过时颜色的显示
$(document).ready(function () { //ready
    /**
     * 设置表单的隔行变色及鼠过亮显
     * .tab表示指定class为tab的表格显示效果
     * 并可为class为tab的表格指定over或alt的样式，该样式已在样式表中定义
     */
    $(".tab tr").mouseover(function () {
        //如果鼠标移到class为tab的表格的tr上时，执行函数
        $(this).addClass("over");
    }).mouseout(function () {
        //给这行添加class值为over，并且当鼠标一出该行时执行函数
        $(this).removeClass("over");
    }) //移除该行的class
    $(".tab tr:even").addClass("alt");
    //给class为tab的表格的偶数行添加class值为alt

    /*
     * 设置按钮响应鼠标的动态效果开始
     * 指定class的样式button为按钮默认显示效果
     * 鼠标经过时切换的样式为but_over，两样式均已在样式表中定义
     */
    $(document).find('input[class="button"]').mouseover(function () {
        //如果鼠标移到class为button的input按钮上时，执行函数
        $(this).addClass("but_over");
    }).mouseout(function () {
        //给这行添加class值为but_over，并且当鼠标移开按钮时执行函数
        $(this).removeClass("but_over");
    }) //移除该按钮的class
    $(this).find('input[class="button"][disabled]').each(function () {
        $(this).removeClass("but_over");
        $(this).addClass("disableButton");
    })

    /*
     * 设置查询条件及修改内容的重置按钮
     */
    $("#resetThis").click(function () {
        $(".search").find("input[type='text']").val("");
        $(".search").find("select").val("");
        $(".search").find('input[type="checkbox"]').attr("checked", false);
        $(".CRselectValue").val("--请选择--");
        $(".SelValue").val("");
    });
});

/* select 联动接口 
 * 
 * 加入控制select的onchange事件
 * 
 * subId：控制select的id
 * subType：控制select类型：long、string
 * targetId：被控制select的id
 * targetType: 被控制select类型：long、string
 * ajaxUrl：
 * 		eg: 'system/Department_ajaxFindSegments.do?dept.id='
 * 		取被控制option的url，返回串，
 *			正常：1,2,3|汉族、回族、满族，
 * 			未取到值：""
 * 			异常："error"
 */
var optionLoading = "<option value=\"-2\">正在加载...</option>";
var headerOptionLong = "<option value=\"-1\">--不限--</option>";
var headerOptionString = "<option value=\"\">--不限--</option>";

function selectInteraction(subId, subType, targetId, targetType, ajaxUrl) {
    if ((subType != 'long' && subType != 'string') || (targetType != 'long' && targetType != 'string')) {
        return alert("subType、targetType只能取long或string！");
    }

    var $sub = $("#" + subId);
    var $target = $("#" + targetId);
    var subValue = $sub.val();

    //控制select选择了不限制，被控select置为不限制
    if ((subType == 'long' && subValue == '-1') || (subType == 'string' && subValue == '')) {
        $target.html(targetType == 'long' ? headerOptionLong : headerOptionString);
        return;
    }

    $target.html(optionLoading);
    $.get(ajaxUrl + subValue, function (ret) {
        $target.html(targetType == 'long' ? headerOptionLong : headerOptionString);

        if (!ret || ret.indexOf("|") < 0) {
            if (ret == 'error') {
                alert("系统错误，请联系系统管理员！");
            }
        } else {
            var kArr = ret.split("|")[0].split(",");
            var vArr = ret.split("|")[1].split(",");
            for (var i = 0; i < kArr.length; i++) {
                $target.append('<option value="' + kArr[i] + '">' + vArr[i] + '</option>');
            }
        }
    });
}