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

$(window).load(function() {
    $('.readmore').readmore({
        collapsedHeight: 16
    });
});

function loadInteractions() {
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
}

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
                }
            });

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