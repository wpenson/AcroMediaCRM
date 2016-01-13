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

// --- Leads Search, Edit, and Delete --- //

Drupal.behaviors.acrocrm_leads = {
    'attach': function(context) {
        $('#lead-edit-modal').on('show.bs.modal', function (event) {
            var button = $(event.relatedTarget);
            var url = button.data('url');

            var modal = $(this);
            modal.find('.modal-content').load(url);
        });

        $('#delete-lead-confirmation-modal').on('show.bs.modal', function (event) {
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

            modal.find('#delete-lead-confirmation-button').click(function() {
                window.location.replace(url);
            });
        });

        $('.lead-filter, #lead-search-button').click(function(event) {
            var target = $(event.currentTarget);
            var url = $('#lead-search').data('url');
            var group = target.data('group');
            var value = '';
            var params = '';

            if (group != null) {
                value = target.data('value');
                params += group + '/' + value;
            }
            else {
                var search_term = $('#lead-search input').val();
                params += 'search';

                if (search_term != '') {
                    params += '/' + search_term;
                }
            }

            $('#leads-list').load(url + params, function(response, status, xhr) {
                if (status == "error") {
                    alert("Sorry but there was an error: " + msg + xhr.status + " " + xhr.statusText);
                    return false;
                }
            });

            if (group != null) {
                $("ul li a[data-group=" + group + "] .lead-search-dropdown-check").remove();
                $("ul li a[data-group=" + group + "][data-value=" + value + "]").prepend("<i class='lead-search-dropdown-check glyphicon glyphicon-ok'></i>");
            }
        });
    }
};