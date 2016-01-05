// --- DRAGGABLE UI --- //

// --- DRAGGABLE UI --- //
$( ".lead-sortable, .lead-list" ).sortable({
    connectWith: ".drag-list",
    receive : function(event, ui) {
        // assume that id for rep is "rep_x"
        var rep_id = $(this).attr("id").split('_')[1];

        // assume that id for lead is "lead_x"
        var lead_id = (ui.item[0].id).split('_')[1];

        $.ajax({
            url: "/acrocrm_leads/assign_lead?lead_id=" + lead_id + "&rep_id=" + rep_id,
            success: function(result) {
                console.log(result);
            }
        })

        return true;
    }
}).disableSelection();

//function myModule_ajax_load() {
//    jQuery("#ajax-target").load("/acrocrm_leads/assign_lead?" + lead_id);
//}