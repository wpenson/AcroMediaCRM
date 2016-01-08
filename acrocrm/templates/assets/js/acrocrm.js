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

// --- Leads Edit and Delete Modal --- //
Drupal.behaviors.acrocrm_leads = {
    'attach': function(context) {
        $('#leadEditModal').on('show.bs.modal', function (event) {
            var button = $(event.relatedTarget);
            var url = button.data('url');

            var modal = $(this);
            modal.find('.modal-content').load(url);
        })

        $('#deleteLeadConfirmationModal').on('show.bs.modal', function (event) {
            var trigger_button = $(event.relatedTarget);
            var url = trigger_button.data('url');

            var modal = $(this);

            //modal.find('#deleteLeadConfirmationButton').click(function() {
            //    $.ajax({
            //        url: url + id,
            //    })
            //    .done(function() {
            //        //modal.modal('hide');
            //        //window.location.assign('overview');
            //    })
            //    .fail(function() {
            //        alert('error');
            //    });
            //});

            modal.find('#deleteLeadConfirmationButton').click(function() {
                window.location.replace(url);
            });
        });
    }
};