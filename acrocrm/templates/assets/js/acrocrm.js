// --- DRAGGABLE UI --- //
$( ".lead-sortable, .lead-list" ).sortable({
    connectWith: ".drag-list",
    receive : function(event, ui) {
        // assume that id for rep is "rep_x"
        var rep_id = $(this).attr("id").split('_')[1];

        // assume that id for lead is "lead_x"
        var lead_id = (ui.item[0].id).split('_')[1];

        $.ajax({
            url: "/acrocrm_leads/assign_lead/"+ lead_id + "/" + rep_id,
            success: function(result) {
                if (($('#rep_' + rep_id + ' .no-assigned-leads')).length > 0) {
                    $('#rep_' + rep_id + ' .no-assigned-leads').remove();
                }
            }
        });

        return true;
    }
}).disableSelection();

$( ".accordion" ).accordion({
    header: "h4",
    collapsible: true,
    active: false
});

function assignPriority(lead_id, element) {

    $.ajax({
        url: "/acrocrm_leads/assign_lead_priority/" + lead_id + "/" + $(element).val()
    });
}

//Drupal.behaviors.acrocrm_leads = {
//    'attach': function(context) {
//        function test_submit() {alert('test'); return false;
//        }
//
//$("#acrocrm-leads-single-lead-form").submit(function(e) {
//    console.log("test");
//    alert("test2");
//    e.preventDefault();
//});
//    }
//}
//
//Drupal.behaviors.acrocrm_leads = {
//    'attach': function(context) {
//        $('#leadEditModal').on('show.bs.modal', function (e) {
//            //if (!data) return e.preventDefault() // stops modal from being shown
//            var button = $(event.relatedTarget);
//            var lead_id = button.data('id');
//
//            var modal = $(this)
//            $.get('/drupal/acrocrm_leads/edit_lead/' + parseInt(lead_id, 10), null, editLead);
//            return false;
//        })
//    }
//}
//
//var editLead = function(response) {
//    //var result = Drupal.parseJson(response);
//    //alert(response);
//    $('#leadEditModal .modal-body').html(response.data);
//}
//Drupal.behaviors.acrocrm_leads = {
//    'attach': function(context) {
//        $('#leadEditModal').on('shown.bs.modal', function (e) {
//            var id = $(e.relatedTarget).data('id');
//            $(this).find('iframe').attr('src', '/drupal/acrocrm_leads/edit_lead/' + id);
//            $(this).modal({show:true});
//        })
//    }
//}