$( "#draggable" ).draggable();

$( "#sortable" ).sortable({
    revert: true
});
$( "#draggable_sortable" ).draggable({
    connectToSortable: "#sortable",
    helper: "clone",
    revert: "invalid"
});