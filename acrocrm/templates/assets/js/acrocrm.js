$(document).ready(function() {
    $( ".list-accordion" ).accordion({
        header: "h5",
        collapsible: true,
        active: false,
        heightStyle: "content",
        activate: function(event, ui) {
            $('.readmore').readmore({
                collapsedHeight: 18
            });
        }
    });

    $( ".header-accordion" ).accordion({
        header: "h4",
        collapsible: true,
        active: false,
        heightStyle: "content"
    });
});

function loadInteractions() {
    $('.readmore').readmore({
        collapsedHeight: 18
    });

    $( ".lead-sortable, .lead-list" ).sortable({
        connectWith: ".drag-list",
        receive : function(event, ui) {

            // ASSIGN LEAD TO REP THAT LEAD WAS DROPPED UNDER

            // assume that id for rep is "rep_x"
            var rep_id = $(this).attr("id").split('_')[1];

            // assume that id for lead is "lead_x"
            var lead_id = (ui.item[0].id).split('_')[1];

            $.ajax({
                url: "/acrocrm_leads/assign_lead/"+ lead_id + "/" + rep_id,
                success: function(result) {
                    updatePriorityText(rep_id, result);

                    if (($('#rep_' + rep_id + ' .no-assigned-leads')).length > 0) {
                        $('#rep_' + rep_id + ' .no-assigned-leads').remove();
                    }
                }
            });

            return true;
        }
    }).disableSelection();
}

function createHubspotContact(lead_id, element) {
    var imgTag = '<img id="loading-gif-' + lead_id + '" alt="loading" src="/acrocrm/templates/assets/images/ajax-loader.gif">';
    $(imgTag).insertBefore($("#" + lead_id).parent());
    $("#" + lead_id).hide();
    $.ajax({
        url: "/acrocrm_hubspot_integration/create_contact/" + lead_id,
        success: function (data) {
            $("#loading-gif-" + lead_id).remove();
            $("#" + lead_id).show();
            var message = data.trim();
            $('#message-container').remove();
            if (message == "success") {
                var prefix = '<div id="message-container" class="row"><div class="col-lg-10 col-md-12"><div class="alert alert-success">';
                var suffix = '<br></div></div></div>';
                $(prefix + 'The HubSpot contact was created successfully' + suffix).insertAfter('#header-row');
                $('#lead_' + lead_id + '_container').remove();

                if ($('.lead-container').length == 0) {
                    $('<div class="no-leads">There are no leads to display.</div>').insertAfter('#delete-lead-confirmation-modal');
                }
            } else {
                var prefix = '<div id="message-container" class="row"><div class="col-lg-10 col-md-12"><div class="alert alert-danger">';
                var suffix = '<br></div></div></div>';

                if (message == "contact_already_exists") {
                    $(prefix + "The contact you are trying to create on HubSpot already exists. " +
                        "This could be due to a duplicate email address." + suffix).insertAfter('#header-row');
                } else if (message == "email_invalid") {
                    $(prefix + "The email address of the contact you are trying to create on HubSpot is invalid. " +
                        "HubSpot has stricter email validation than AcroCRM." + suffix).insertAfter('#header-row');
                } else if (message == "email_invalid") {
                    $(prefix + "The the contact you are trying to create could not be found. " +
                        "Refresh the page and try again" + suffix).insertAfter('#header-row');
                } else {
                    $(prefix + message + suffix).insertAfter('#header-row');
                }
            }
        },
        dataType: 'text'
    });
}

function assignPriority(lead_id, element) {
    $.ajax({
        url: "/acrocrm_leads/assign_lead_priority/" + lead_id + "/" + $(element).val()
    });
}

function updatePriorityText(rep_id, priority) {
    if (priority === '') {
        var text = $('#unassigned-' + rep_id).text();
        text = text.split(' ');
        var new_priority = parseInt(text[1]) + 1;
        $('#unassigned-' + rep_id).text("Unassigned: " + new_priority);
    }

    else if (priority === 'low') {
        var text = $('#low-' + rep_id).text();
        text = text.split(' ');
        var new_priority = parseInt(text[1]) + 1;
        $('#low-' + rep_id).text("Low: " + new_priority);
    }

    else if (priority === 'medium') {
        var text = $('#med-' + rep_id).text();
        text = text.split(' ');
        var new_priority = parseInt(text[1]) + 1;
        $('#med-' + rep_id).text("Medium: " + new_priority);
    }

    else if (priority === 'high') {
        var text = $('#high-' + rep_id).text();
        text = text.split(' ');
        var new_priority = parseInt(text[1]) + 1;
        $('#high-' + rep_id).text("High: " + new_priority);
    }
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

        function loadLeadList(event) {
            var target = $(event.currentTarget);
            var url = $('#lead-search').data('url');
            var group = target.data('group');
            var lead_search_box = $('#lead-search input');
            var value = '';
            var params = '';

            if (group != null) {
                value = target.data('value');
                params += group + '/' + value;
            }
            else {
                var search_term = lead_search_box.val();
                params += 'search';

                if (search_term != '') {
                    params += '/' + $.trim(search_term).replace(/ /g,"+");
                }
            }

            var spinner_opts = {
                lines: 13 // The number of lines to draw
                , length: 28 // The length of each line
                , width: 14 // The line thickness
                , radius: 42 // The radius of the inner circle
                , scale: 0.25 // Scales overall size of the spinner
                , corners: 0 // Corner roundness (0..1)
                , color: '#000' // #rgb or #rrggbb or array of colors
                , opacity: 0.25 // Opacity of the lines
                , rotate: 0 // The rotation offset
                , direction: 1 // 1: clockwise, -1: counterclockwise
                , speed: 1.5 // Rounds per second
                , trail: 60 // Afterglow percentage
                , fps: 20 // Frames per second when using setTimeout() as a fallback for CSS
                , zIndex: 2e9 // The z-index (defaults to 2000000000)
                , className: 'spinner' // The CSS class to assign to the spinner
                , top: '190%' // Top position relative to parent
                , left: '50%' // Left position relative to parent
                , shadow: false // Whether to render a shadow
                , hwaccel: false // Whether to use hardware acceleration
                , position: 'absolute' // Element positioning
            };

            var spinner = new Spinner(spinner_opts).spin();
            var lead_list = $('#leads-list');
            lead_list.html(spinner.el);

            lead_list.load(url + params, function(response, status, xhr) {
                if (status == "error") {
                    $('#leads-list').html("<div class='lead-list-message-div'>Sorry but there was an error: " + xhr.status + " " + xhr.statusText + "</div>");
                    return false;
                }
                else if (status == "success") {
                    loadInteractions();

                    if (group != null) {
                        if (group != 'sort-order') {
                            $("ul li a[data-group=" + group + "] .lead-search-dropdown-check").remove();
                            $("ul li a[data-group=" + group + "][data-value=" + value + "]").prepend("<i class='lead-search-dropdown-check glyphicon glyphicon-ok'></i>");
                        }
                        else {
                            if (value == 'asc') {
                                $("[data-group='sort-order'][data-value=desc]").removeClass("active");
                                $("[data-group='sort-order'][data-value=asc]").addClass("active");
                            }
                            else {
                                $("[data-group='sort-order'][data-value=asc]").removeClass("active");
                                $("[data-group='sort-order'][data-value=desc]").addClass("active");
                            }
                        }

                        if (group == 'search-field') {
                            lead_search_box.attr('placeholder', "Search by " + value);
                        }
                    }
                }
            });
        }

        var timeout_thread = null;
        var previous_val = '';
        $('#lead-search input').on('keyup', function(e) {
            if (e.keyCode == '13') {
                loadLeadList(e);
                return;
            }

            if ($.trim($(this).val()) == '' && $.trim(previous_val) == '') {
                previous_val = $(this).val();
                return;
            }
            previous_val = $(this).val();

            clearTimeout(timeout_thread);
            timeout_thread = setTimeout(function() { loadLeadList(e); }, 100);
        });

        $(document).ready(function(e) {
            loadLeadList(e);
        });

        $('.lead-filter, #lead-search-button').on('click', function(e) {
            loadLeadList(e);
            e.stopPropagation();
        });
    }
};