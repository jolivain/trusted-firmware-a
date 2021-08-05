$(document).ready(function() {
    $(".collapsible.collapsed > :not(:first-child)").hide();

    $(".collapsible > :first-child").click(function() {
        var container = $(this).parent();
        var children = container.children().not(":first-child");

        container.toggleClass("collapsed", 400);
        children.toggle(400);
    })
});
