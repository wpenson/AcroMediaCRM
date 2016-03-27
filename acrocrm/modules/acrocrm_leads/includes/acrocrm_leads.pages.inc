<?php

/**
 * Page callback for the leads overview page. Generates the page with the leads list container and the sales rep list.
 *
 * @return string  Themed page
 */
function acrocrm_leads_overview_page() {
  $page['content']['leads'] = array(
    '#markup' => acrocrm_leads_generate_leads_list_container_html()
  );

  $page['content']['reps'] = array(
    '#markup' => acrocrm_leads_get_sales_reps()
  );

  return theme('acrocrm_leads_home', array('page' => $page));
}

function acrocrm_leads_my_leads_page() {
  $page['content']['header'] = array(
    '#markup' => '<div id="header-row" class="row">
                    <div class="col-lg-12">
                      <h1 class="page-header">' . t('My Leads') . '</h1>
                    </div>
                  </div>'
  );

  // Need to initialize a placeholder so this displays before the page content.
  $page['content']['messages']['#markup'] = '';

  $page['content']['leads$'] = array(
    '#markup' => acrocrm_leads_generate_leads_list_container_html('my_leads')
  );

  // The messages appear after the form has been loaded.
  $page['content']['messages']['#markup'] = acrocrm_get_message_markup();

  return theme('acrocrm_leads_home', array('page' => $page));
}

/**
 * Ajax callback that assigns a lead to a sales representative.
 *
 * @param $rep_id   int   ID of the sales representative
 * @param $lead_id  int   ID of the lead
 */
function acrocrm_leads_assign_lead_ajax($rep_id, $lead_id) {
  // Check to see if the lead is already assigned to someone
  $check_exists = db_select('assigned_leads', 'al')
    ->fields('al')
    ->condition('lead_id', $lead_id, '=')
    ->execute();

  // If lead exists just update assignment
  if ($check_exists->rowCount() > 0) {
    db_update('assigned_leads')
      ->fields(array(
        'uid' => $rep_id
      ))
      ->condition('lead_id', $lead_id, '=')
      ->execute();
  }
  // Else insert the assigned lead and assign to correct sales rep
  else {
    db_insert('assigned_leads')
      ->fields(array(
        'uid' => $rep_id,
        'lead_id' => $lead_id
      ))
      ->execute();
  }

  // Regenerate leads list and print it out
  $leads_list = '';
  acrocrm_leads_generate_sales_rep_lead_list($rep_id, $leads_list);
  echo $leads_list;
}

/**
 * Ajax callback that un-assigns a lead [from a sales representative].
 *
 * @param $lead_id  int  ID of the lead
 */
function acrocrm_leads_unassign_lead_ajax($lead_id) {
  // Delete the lead assignments from the assignment table
  db_delete('assigned_leads')
    ->condition('lead_id', $lead_id)
    ->execute();
}

/**
 * Ajax callback that gets the priority of a lead.
 *
 * @param $lead_id  int  ID of the Lead
 */
function acrocrm_leads_get_lead_priority_ajax($lead_id) {
  $check_exists = db_select('leads', 'l')
    ->fields('l')
    ->condition('lead_id', $lead_id, '=')
    ->execute();

  if ($check_exists->rowCount() > 0) {
    $get_priority = db_select('leads', 'l')
      ->fields('l', array('priority'))
      ->condition('lead_id', $lead_id, '=')
      ->execute();

    $priority = $get_priority->fetchAssoc();
    if (strlen($priority['priority']) == 0) {
      echo 'unassigned';
    }
    else {
      echo $priority['priority'];
    }
  }
  else {
    echo 'error'; // TODO: What does this do?
  }
}

/**
 * Ajax callback that sets the priority of a lead.
 *
 * @param $priority string  Priority of a lead. Can be either 'high', 'medium', 'low', or 'unassigned'
 * @param $lead_id  int     ID of the lead
 */
function acrocrm_leads_set_lead_priority_ajax($priority, $lead_id) {
  $check_exists = db_select('leads', 'l')
    ->fields('l')
    ->condition('lead_id', $lead_id, '=')
    ->execute();

  if ($priority === 'unassigned') {
    $priority = '';
  }

  if ($check_exists->rowCount() > 0) {
    db_update('leads')
      ->fields(array(
        'priority' => $priority
      ))
      ->condition('lead_id', $lead_id, '=')
      ->execute();
  }
}

/**
 * Ajax callback that filters the leads and then echos the lead list. The options for the filter are set by a group and then a value for
 * that group. The filter options are not set all at once. They are set one at a time.
 *
 * The allowed groups and corresponding values are as follows:
 * -'search-field' = The content to search by
 *  -'name' = Name including first and last
 *  -'email' = Email address
 *  -'comments' = Comments
 *  -'phone' = Phone number
 * -'show' = Show the leads based on their assignment
 *  -'all' = All of the leads
 *  -'unassigned' = Unassigned leads
 *  -'assigned' = Assigned leads
 *  -'deleted' = Deleted leads
 * -'sort' = The sort order of the elements
 *  -'created-time' = Time that a lead was created
 *  -'modified-time' = Time that a lead was modified
 *  -'priority' = Priority of a lead
 *  -'name' = Name of the Lead by first name
 *  -'email' = Email
 * -'sort-order' = The sort order
 *  -'asc' = Ascending
 *  -'desc' = Descending
 * -'search' = Search value
 *  -Any string
 *
 * @param $group string The group of the filter option to change.
 * @param $value string The option in the filter group to change to.
 */
function acrocrm_leads_filter_leads_ajax($first_record = 0, $last_record = 6, $group = NULL, $value = NULL) {
  // Allowed values used for validation
  $valid_search_field_values = ['name', 'email', 'comments', 'phone'];
  $valid_show_values = ['all', 'committed', 'unassigned', 'assigned', 'deleted'];
  $valid_sort_values = ['created_time', 'modified_time', 'priority', 'name', 'email'];

  // Store the filter preferences in the session
  drupal_session_start();

  if ($group != NULL) {
    if ($group == 'search-field' && in_array($value, $valid_search_field_values)) {
      $_SESSION['lead_search_pref'] = $value;
    }
    else if ($group == 'show' && in_array($value, $valid_show_values)) {
      $_SESSION['lead_show_pref'] = $value;
    }
    else if ($group == 'sort' && in_array($value, $valid_sort_values)) {
      $_SESSION['lead_sort_pref'] = $value;
    }
    else if ($group == 'sort-order' && ($value == 'asc' || $value == 'desc')) {
      $_SESSION['lead_sort_order_pref'] = $value;
    }
    else if ($group == 'search') {
      $_SESSION['lead_search_val'] = $value;
    }
  }

  // Set defaults for the session variables
  if (!isset($_SESSION['lead_search_pref'])) {
    $_SESSION['lead_search_pref'] = 'name';
  }

  if (!isset($_SESSION['lead_show_pref'])) {
    $_SESSION['lead_show_pref'] = 'unassigned';
  }

  if (!isset($_SESSION['lead_sort_pref'])) {
    $_SESSION['lead_sort_pref'] = 'created_time';
  }

  if (!isset($_SESSION['lead_sort_order_pref'])) {
    $_SESSION['lead_sort_order_pref'] = 'asc';
  }

  if (!isset($_SESSION['lead_search_val'])) {
    $_SESSION['lead_search_val'] = '';
  }

  // Build query
  $leads_query = db_select('leads', 'l');
  $leads_query->fields('l');
  $assigned_fields = FALSE;

  if ($_SESSION['lead_show_pref'] == 'committed') {
    $leads_query->condition('l.status', 'COMMITTED', '=');
    $leads_query->innerJoin('assigned_leads', 'al', 'l.lead_id = al.lead_id');
    $leads_query->fields('al', array('uid'));
    $assigned_fields = TRUE;
  }
  else if ($_SESSION['lead_show_pref'] == 'unassigned') {
    $leads_query->condition('l.status', 'DELETED', '<>');
    $leads_query->leftJoin('assigned_leads', 'al', 'l.lead_id = al.lead_id');
    $leads_query->isNull('al.lead_id');
  }
  else if ($_SESSION['lead_show_pref'] == 'assigned') {
    $leads_query->condition('l.status', 'COMMITTED', '<>');
    $leads_query->innerJoin('assigned_leads', 'al', 'l.lead_id = al.lead_id');
    $leads_query->fields('al', array('uid'));
    $assigned_fields = TRUE;
  }
  else if ($_SESSION['lead_show_pref'] == 'deleted') {
    $leads_query->condition('l.status', 'DELETED', '=');
  }
  else if ($_SESSION['lead_show_pref'] == 'all') {
    $leads_query->leftJoin('assigned_leads', 'al', 'l.lead_id = al.lead_id');
    $leads_query->fields('al', array('uid'));
    $assigned_fields = TRUE;
  }

  if ($_SESSION['lead_search_val'] != '') {
    if ($_SESSION['lead_search_pref'] == 'name') {
      $name = explode(' ', $_SESSION['lead_search_val']);

      $or = db_or();
      $or->condition('l.name', '%' . $name[0] . '%', 'LIKE');
      $or->condition('l.firstname', '%' . $name[0] . '%', 'LIKE');
      $leads_query->condition($or);

      if (isset($name[1])) {
        $or = db_or();
        $or->condition('l.name', '%' . $name[1] . '%', 'LIKE');
        $or->condition('l.firstname', '%' . $name[1] . '%', 'LIKE');
        $leads_query->condition($or);
      }
    }
    else {
      $leads_query->condition('l.' . $_SESSION['lead_search_pref'], '%' . $_SESSION['lead_search_val'] . '%', 'LIKE');
    }
  }

  if ($_SESSION['lead_sort_pref'] == 'priority') {
    $leads_query->addExpression("CASE WHEN l.priority = 'high' THEN 1
                                      WHEN l.priority = 'medium' THEN 2
                                      WHEN l.priority = 'low' THEN 3
                                      WHEN l.priority = '' THEN 4
                                      ELSE l.priority END",
                                'order_col');

    $leads_query->orderBy('order_col', strtoupper($_SESSION['lead_sort_order_pref']));
  }
  else {
    $leads_query->orderBy('l.' . $_SESSION['lead_sort_pref'], strtoupper($_SESSION['lead_sort_order_pref']));
  }

  $leads_query->range($first_record, $last_record);
  $leads_query = $leads_query->execute();
  $leads = array();

  if ($leads_query->rowCount() > 0) {
    while ($lead = $leads_query->fetchAssoc()) {
      $arr_entry = array(
        'lead_id' => $lead['lead_id'],
        'firstname' => $lead['firstname'],
        'name' => $lead['name'],
        'email' => $lead['email'],
        'phone' => $lead['phone'],
        'comments' => $lead['comments'],
        'priority' => $lead['priority'],
        'status' => $lead['status']
      );

      if ($assigned_fields === TRUE) {
        $arr_entry['rep_id'] = $lead['uid'];
      }

      $leads[] = $arr_entry;
    }

    echo acrocrm_leads_generate_leads_list_html($leads);
  }
  else {
    echo '<li class="list-group-item lead-list-error-box">' . t('No Leads Found') . '</li>';
  }
}

/**
 * Ajax callback that creates the lead edit modal/pop-up with the lead edit form.
 *
 * @param $lead_id        int     ID of the lead to edit
 * @param $redirect_page  string  The page to redirect to after editing the lead
 */
function acrocrm_leads_edit_lead_ajax($lead_id, $redirect_page) {
  $form = drupal_get_form('acrocrm_leads_edit_lead_form');

  $lead = db_select('leads', 'l')
    ->fields('l')
    ->condition('lead_id', $lead_id, '=')
    ->execute()
    ->fetchAssoc();

  $form['firstname']['#value'] = $lead['firstname'];
  $form['name']['#value'] = $lead['name'];
  $form['email']['#value'] = $lead['email'];
  $form['phone']['#value'] = $lead['phone'];
  $form['comments']['#value'] = $lead['comments'];
  $form['priority']['#value'] = $lead['priority'];
  $form['redirect_page']['#value'] = $redirect_page;

  $assigned_salesrep = db_select('assigned_leads', 'al')
    ->fields('al', array('uid'))
    ->condition('lead_id', $lead_id, '=')
    ->execute()
    ->fetchAssoc();

  if (isset($assigned_salesrep['uid'])) {
    $form['assign']['#value'] = $assigned_salesrep['uid'];
  }

  $form['lead_id']['#value'] = $lead_id;

  $form["#prefix"] =
    '<div class="modal-header">
      <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
      <h4 class="modal-title" id="editLead">' . t('Edit Lead') . '</h4>
    </div>
    <div class="modal-body">';

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save Changes'),
    '#prefix' => '</div><div class="modal-footer"><button type="button" class="btn btn-default" data-dismiss="modal">' . t('Close') . '</button>',
    '#suffix' => '</div>',
    '#weight' => 7,
    '#attributes' => array(
      'class' => array('btn', 'btn-primary'),
    ),
  );

  echo drupal_render($form);
}

/**
 * Ajax callback that marks a lead in the database as deleted.
 *
 * @param $lead_id        int     ID of the lead to delete
 * @param $redirect_page  string  The page to redirect to after deleting the lead
 */
function acrocrm_leads_delete_lead_ajax($lead_id) {
  db_update('leads')
    ->fields(array('status' => 'DELETED'))
    ->condition('lead_id', $lead_id, '=')
    ->execute();

  db_delete('assigned_leads')
    ->condition('lead_id', $lead_id, '=')
    ->execute();

  drupal_goto($path = '/acrocrm_leads/overview');
}

/**
 * Ajax callback that marks a lead in the database as uncommitted (ie. recovers the lead).
 *
 * @param $lead_id        int     ID of the lead to recover
 * @param $redirect_page  string  The page to redirect to after recovering the lead
 */
function acrocrm_leads_recover_lead_ajax($lead_id) {
  db_update('leads')
    ->fields(array('status' => 'UNCOMMITTED'))
    ->condition('lead_id', $lead_id, '=')
    ->execute();

  drupal_goto($path = '/acrocrm_leads/overview');
}

/**
 * Creates the container for the leads list.
 *
 * @TODO: This would warrant a template...
 *
 * @return string HTML container
 */
function acrocrm_leads_generate_leads_list_container_html($page = 'overview') {
  $search_div = '';
  $lead_create_modal = '';
  if ($page === 'overview') {
    $search_div .=
      '<div id="lead-search" class="filter-header" data-url="' . base_path() . 'acrocrm_leads/filter_leads/">
        <button type="button" id="create-new-lead-button" class="btn btn-default" data-tooltip="tooltip" title="' . t('Create New Lead') . '" data-url="' . base_path() . 'acrocrm_leads/new_lead/" data-toggle="modal" data-target="#lead-create-modal">
          <i class="glyphicon glyphicon-plus" style="color:#009900;"></i>
        </button>
        <div class="input-group">
          <input type="text" class="form-control" aria-label="..." placeholder="">
          <div class="input-group-btn">
            <button type="button" id="lead-search-button" class="btn btn-default">
              <i class="glyphicon glyphicon-search"></i>
            </button>
            <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
              <span class="caret"></span>
            </button>
            <ul class="dropdown-menu dropdown-menu-right" style="z-index: 1;">
              <li>&nbsp;&nbsp;<b>' . t('Search by') . '</b></li>
              <li><a href="#" class="lead-filter" data-group="search-field" data-value="name">' . t('Name') . '</a></li>
              <li><a href="#" class="lead-filter" data-group="search-field" data-value="email">' . t('Email') . '</a></li>
              <li><a href="#" class="lead-filter" data-group="search-field" data-value="phone">' . t('Phone') . '</a></li>
              <li><a href="#" class="lead-filter" data-group="search-field" data-value="comments">' . t('Comments') . '</a></li>
  
              <li role="separator" class="divider"></li>
  
              <li>&nbsp;&nbsp;<b>' . t('Show') . '</b></li>
              <li><a href="#" class="lead-filter" data-group="show" data-value="all">' . t('All') . '</a></li>
              <li><a href="#" class="lead-filter" data-group="show" data-value="committed">' . t('Committed') . '</a></li>
              <li><a href="#" class="lead-filter" data-group="show" data-value="assigned">' . t('Assigned') . '</a></li>
              <li><a href="#" class="lead-filter" data-group="show" data-value="unassigned">' . t('Unassigned') . '</a></li>
              <li><a href="#" class="lead-filter" data-group="show" data-value="deleted">' . t('Deleted') . '</a></li>
  
              <li role="separator" class="divider"></li>
  
              <li>
                &nbsp;&nbsp;
                <b>' . t('Sort by') . '</b>
                &nbsp;
                <span class="btn-group btn-group-xs pull-right" role="group" style="margin-right: 5px;">
                  <button type="button" class="lead-filter btn btn-default" data-group="sort-order" data-value="asc">' . t('Asc') . '</button>
                  <button type="button" class="lead-filter btn btn-default" data-group="sort-order" data-value="desc">' . t('Desc') . '</button>
                </span>
              </li>
              <li><a href="#" class="lead-filter" data-group="sort" data-value="created_time">' . t('Date Created') . '</a></li>
              <li><a href="#" class="lead-filter" data-group="sort" data-value="modified_time">' . t('Date Modified') . '</a></li>
              <li><a href="#" class="lead-filter" data-group="sort" data-value="priority">' . t('Priority') . '</a></li>
              <li><a href="#" class="lead-filter" data-group="sort" data-value="name">' . t('Last Name') . '</a></li>
              <li><a href="#" class="lead-filter" data-group="sort" data-value="email">' . t('Email') . '</a></li>
            </ul>
          </div>
        </div>
      </div>';

    drupal_session_start();

    // Set defaults
    if (!isset($_SESSION['lead_search_pref'])) {
      $_SESSION['lead_search_pref'] = 'name';
    }

    if (!isset($_SESSION['lead_show_pref'])) {
      $_SESSION['lead_show_pref'] = 'unassigned';
    }

    if (!isset($_SESSION['lead_sort_pref'])) {
      $_SESSION['lead_sort_pref'] = 'created_time';
    }

    if (!isset($_SESSION['lead_sort_order_pref'])) {
      $_SESSION['lead_sort_order_pref'] = 'asc';
    }

    if (!isset($_SESSION['lead_search_val'])) {
      $_SESSION['lead_search_val'] = '';
    }

    // This is used to avoid having a complicated string for $search_div.
    $search_pref_js =
      'jQuery(document).ready(function() {
        $("ul li a[data-group=\'search-field\'][data-value=\'' . $_SESSION['lead_search_pref'] . '\']").prepend("<i class=\'lead-search-dropdown-check glyphicon glyphicon-ok\'></i>");
        $("ul li a[data-group=\'show\'][data-value=\'' . $_SESSION['lead_show_pref'] . '\']").prepend("<i class=\'lead-search-dropdown-check glyphicon glyphicon-ok\'></i>");
        $("ul li a[data-group=\'sort\'][data-value=\'' . $_SESSION['lead_sort_pref'] . '\']").prepend("<i class=\'lead-search-dropdown-check glyphicon glyphicon-ok\'></i>");
        $("[data-group=\'sort-order\'][data-value=\'' . $_SESSION['lead_sort_order_pref'] . '\']").addClass("active");
        $("#lead-search input").attr("placeholder", "Search by ' . $_SESSION['lead_search_pref'] . '");
      });';

    drupal_add_js($search_pref_js, 'inline');

  $lead_create_modal =
    '<div class="modal fade" id="lead-create-modal" tabindex="-1" role="dialog" aria-labelledby="new-lead">
			<div class="vertical-alignment-helper">
				<div class="modal-dialog modal-lg vertical-align-center" role="document">
					<div class="modal-content"></div>
				</div>
			</div>
		</div>';
  }

  $leads_edit_modal =
    '<div class="modal fade" id="lead-edit-modal" tabindex="-1" role="dialog" aria-labelledby="edit-lead">
			<div class="vertical-alignment-helper">
				<div class="modal-dialog modal-lg vertical-align-center" role="document">
					<div class="modal-content"></div>
				</div>
			</div>
		</div>';

  $delete_lead_confirmation_modal =
    '<div class="modal fade" id="delete-lead-confirmation-modal" tabindex="-1" role="dialog" aria-labelledby="delete-lead">
			<div class="vertical-alignment-helper">
				<div class="modal-dialog modal-sm vertical-align-center" role="document">
					<div class="modal-content">
						<div class="modal-header">
							<button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
							<h4 class="modal-title" id="edit-lead">' . t('Delete Lead') . '</h4>
						</div>
						<div class="modal-body">'
							. t('Are you sure you want to delete this lead?') .
						'</div>
						<div class="modal-footer">
							<button type="button" class="btn btn-default" data-dismiss="modal">' . t('No') . '</button>
							<button type="button" id="delete-lead-confirmation-button" class="btn btn-danger">' . t('Yes') . '</button>
						</div>
					</div>
				</div>
			</div>
		</div>';

  $leads_div = '';

  if ($page === 'overview') {
    $leads_list =
      '
      <div class="leads-list-spinner">
        <img src="' . base_path() . drupal_get_path('module', 'acrocrm') . '/templates/assets/images/spinner.gif">
      </div><ul id="leads-list" class="list-group"></ul>';

    $leads_div =
      '<div class="col-md-6" style="margin-top: 15px;">' .
        $search_div . $lead_create_modal . $leads_edit_modal . $delete_lead_confirmation_modal . $leads_list .
      '</div>';
  }
  else if ($page === 'my_leads') {
    global $user;

    $al_query = db_select('leads', 'l');
    $al_query->fields('l');
    $al_query->innerJoin('assigned_leads', 'al', 'al.lead_id = l.lead_id');
    $al_query->condition('al.uid', $user->uid, '=');
    $al_query->condition('l.status', 'COMMITTED', '!=');
    $al_query->condition('l.status', 'DELETED', '!=');
    $al_leads = $al_query->execute();

    $leads = array();

    if ($al_leads->rowCount() > 0) {
      while ($lead = $al_leads->fetchAssoc()) {
        $arr_entry = array(
          'lead_id' => $lead['lead_id'],
          'firstname' => $lead['firstname'],
          'name' => $lead['name'],
          'email' => $lead['email'],
          'phone' => $lead['phone'],
          'comments' => $lead['comments'],
          'priority' => $lead['priority'],
          'status' => $lead['status']
        );

        $leads[] = $arr_entry;
      }

      $leads_list =
        '<ul id="my-leads-list" class="list-group">'
          . acrocrm_leads_generate_leads_list_html($leads, 'my_leads') .
        '</ul>';
    }
    else {
      $leads_list = '<ul id="my-leads-list" class="list-group">You have no assigned leads</ul>';
    }

    $leads_div =
      '<div class="col-lg-10 col-md-12" style="margin-top: 15px;">' .
        $leads_edit_modal . $delete_lead_confirmation_modal . $leads_list .
      '</div>';
  }

  return $leads_div;
}

/**
 * Creates the list of leads.
 *
 * @param $leads  array   Array of leads
 * @return string           HTML list of leads
 */
function acrocrm_leads_generate_leads_list_html(&$leads, $page = 'overview') {
  $leads_list = '';

  foreach ($leads as $lead) {
    if (strlen($lead['firstname']) > 0) {
      $lead_name = $lead['firstname'] . ' ' . $lead['name'];
    }
    else {
      $lead_name = $lead['name'];
    }

    if ($page === 'overview') {
      $assigned = (isset($lead['rep_id']) && $lead['rep_id'] != NULL) ? TRUE : FALSE;
      $lead_status = '';
      $corner_indicator = '';

      if ($assigned === TRUE) {
        $lead_status = t('Assigned');
        $corner_indicator = 'yellow-indicator';
      }

      switch ($lead['status']) {
        case 'COMMITTED':
          $lead_status = t('Assigned and Committed');
          $corner_indicator = 'green-indicator';
          break;
        case 'UNCOMMITTED':
          if ($assigned !== TRUE) {
            $lead_status = t('Not Assigned and Not Committed');
            $corner_indicator = 'grey-indicator';
          } else {
            $lead_status = t('Assigned but Not Committed');
          }
          break;
        case 'DELETED':
          $lead_status = t('Deleted');
          $corner_indicator = 'red-indicator';
          break;
        default:
          continue;
      }
    }

    $leads_list .= '<li data-lead-id="' . $lead['lead_id'] . '" class="list-group-item' . (($page === 'overview' && $assigned !== TRUE && $lead['status'] !== 'DELETED') ? ' unassigned-lead' : '') . '" data-url="' . base_path() . 'acrocrm_leads/" href="javascript:void(0)">';

    $leads_list .= '<div class="pull-right text-right">';
    $leads_list .= '<div class="btn-group" role="group" aria-label="...">';

    if ($lead['status'] !== 'DELETED' || $page === 'my_leads') {
      $leads_list .= '<button data-tooltip="tooltip" title="' . t('Edit Lead') . '" type="button" class="edit-lead-button btn btn-primary" data-url="' . base_path() . 'acrocrm_leads/edit_lead/' . $lead['lead_id'] . '" data-toggle="modal" data-target="#lead-edit-modal"><span class="glyphicon glyphicon-pencil"></span></button>';
      $leads_list .= '<button data-tooltip="tooltip" title="' . t('Delete Lead') . '" type="button" class="delete-lead-button btn btn-danger" data-url="' . base_path() . 'acrocrm_leads/delete_lead/' . $lead['lead_id'] . '" data-toggle="modal" data-target="#delete-lead-confirmation-modal"><span class="glyphicon glyphicon-trash"></span></button>';
    }
    else {
      $leads_list .= '<button data-tooltip="tooltip" title="' . t('Recover Lead') . '" type="button" class="recover-lead-button btn btn-success" data-url="' . base_path() . 'acrocrm_leads/recover_lead/' . $lead['lead_id'] . '"><span class="glyphicon glyphicon-repeat"></span></button>';
    }

    $leads_list .= '</div>';
    $leads_list .= '</div>';

    $leads_list .= '<h4 class="list-group-item-heading">';
    $leads_list .= $lead_name;
    if ($page === 'overview') {
      $leads_list .= ' <span data-tooltip="tooltip" title="' . $lead_status . '" class="corner-indicator ' . $corner_indicator . '"></span>';
    }
    $leads_list .= '</h4>';

    if (isset($lead['email']) && $lead['email'] != '') {
      $leads_list .= '<p class="list-group-item-text"><strong>' . t('Email') . ': </strong>' . $lead['email'] . '</p>';
    }

    if (isset($lead['phone']) && $lead['phone'] != '') {
      $leads_list .= '<p class="list-group-item-text"><strong>' . t('Phone') . ': </strong>' . $lead['phone'] . '</p>';
    }

    $priority = $lead['priority'];
    $leads_list .= '<p class="priority-radio-btn list-group-item-text"><strong>' . t('Priority') . ': </strong>';
    if ($priority === '') {
      $priority = 'none';
    }
    $leads_list .=
      '<label class="c-input c-radio">
        <input class="high-priority" type="radio" value="high-priority" name="' . $lead['lead_id'] . '" ' . (($priority === 'high') ? 'checked' : '') . '>
        <span data-tooltip="tooltip" title="' . t('High Priority') . '" class="c-indicator"></span>
      </label>';
    $leads_list .=
      '<label class="c-input c-radio">
        <input class="med-priority" type="radio" value="med-priority" name="' . $lead['lead_id'] . '" ' . (($priority === 'medium') ? 'checked' : '') . '>
        <span data-tooltip="tooltip" title="' . t('Medium Priority') . '" class="c-indicator"></span>
      </label>';
    $leads_list .=
      '<label class="c-input c-radio">
        <input class="low-priority" type="radio" value="low-priority" name="' . $lead['lead_id'] . '" ' . (($priority === 'low') ? 'checked' : '') . '>
        <span data-tooltip="tooltip" title="' . t('Low Priority') . '" class="c-indicator"></span>
      </label>';
    $leads_list .=
      '<label class="c-input c-radio">
        <input class="no-priority" type="radio" value="no-priority" name="' . $lead['lead_id'] . '" ' . (($priority === 'none') ? 'checked' : '') . '>
        <span data-tooltip="tooltip" title="' . t('No Priority') . '" class="c-indicator"></span>
      </label>';
    $leads_list .= '</p>';

    if (isset($lead['comments']) && $lead['comments'] != '') {
      $leads_list .= '<p class="list-group-item-text readmore"><strong>' . t('Comments') . ': </strong>' . $lead['comments'] . '</p>';
    }
    $leads_list .= '</li>';
  }

  return $leads_list;
}

/**
 * Get a list of all sales reps and create html to display sales reps
 *
 * @return string
 */
function acrocrm_leads_get_sales_reps() {
  // TODO: Add buttons/dropdowns and functionality to the filter bar. Need date range, collapse all, open all, send to hubspot, email, etc.
  $filter_bar =
    '<div id="lead-search" class="filter-header" data-url="' . base_path() . 'acrocrm_leads/filter_leads/">
      <div class="input-group input-group-btns">'
        . (module_exists('acrocrm_hubspot_integration') && variable_get('automatic_creation') == TRUE ? '<button id="commit-leads-button" type="button" class="btn btn-primary" data-module-base-path="' . drupal_get_path('module', 'acrocrm') . '" data-url-base-path="' . base_path() . '">' . t('Commit Leads') . '</button>' : '')
        . '<button id="expand-all" type="button" class="btn btn-primary pull-right expand-repdiv">' . t('Expand All') . '</button>'
    // TODO: Add more functions.
    . '  </div>
    </div>';

  $users = entity_load('user');
  $rep_div = '';

  $role = user_role_load_by_name('sales rep');
  $role_id = $role ? $role->rid : NULL;
  $result = db_select('users_roles', 'ur')
    ->fields('ur')
    ->condition('uid', 1, '=')
    ->condition('rid', $role_id, '=')
    ->execute()
    ->fetchAssoc();

  foreach ($users as $user) {
    if (($user->uid != 1 && user_access('sales rep', $user)) || ($user->uid == 1 && $result > 0)) {
      $leads_list = '';
      $priorities = acrocrm_leads_generate_sales_rep_lead_list($user->uid, $leads_list);

      $rep_div .= '<li data-rep-id="' . $user->uid . '" class="list-group-item sales-rep">';
      $rep_div .= '<h4 class="list-group-item-heading sorting-disabled sales-rep-closed"><i class="fa fa-angle-right fa-fw"></i><img class="sales-rep-spinner" style="margin-right:5px;" src="' . base_path() . drupal_get_path('module', 'acrocrm') . '/templates/assets/images/spinner.gif">' . $user->name;
      $rep_div .= '<span class="pull-right">';
      $rep_div .= '<span data-tooltip="tooltip" title="' . t('High Priority') . '" id="high-' . $user->uid .'" class="high-priority priority-indicator">' . $priorities['high'] . '</span>';
      $rep_div .= '<span data-tooltip="tooltip" title="' . t('Medium Priority') . '" id="med-' . $user->uid .'" class="med-priority priority-indicator">' . $priorities['med'] . '</span>';
      $rep_div .= '<span data-tooltip="tooltip" title="' . t('Low Priority') . '" id="low-' . $user->uid .'" class="low-priority priority-indicator">' . $priorities['low'] . '</span>';
      $rep_div .= '<span data-tooltip="tooltip" title="' . t('No Priority') . '" id="unassigned-' . $user->uid .'" class="priority-indicator">' . $priorities['unassigned'] . '</span>';
      $rep_div .= '</span>';
      $rep_div .= '</h4>';

      $rep_div .= '<div>';
      $rep_div .= '<ul class="list-group sales-rep-lead-list">';
      $rep_div .= $leads_list;
      $rep_div .= '</ul>';
      $rep_div .= '</div>';
      $rep_div .= '</li>';
    }
  }

  $sales_reps =
    '<div class="col-md-6" style="margin-top: 15px;">' .
      $filter_bar . '<ul class="list-group sales-rep-list">' . $rep_div . '</ul>
    </div>';

  return $sales_reps;
}

function acrocrm_leads_generate_sales_rep_lead_list($sales_rep_user_id, &$leads_list) {
  $assigned_leads = db_select('assigned_leads', 'lead_id')
    ->fields('lead_id')
    ->condition('uid', $sales_rep_user_id, '=')
    ->execute();

  $lead_ids = array();

  while ($lead_id = $assigned_leads->fetchAssoc()) {
    array_push($lead_ids, $lead_id["lead_id"]);
  }

  // keep track of the priority for each type of lead -> will be displayed next to rep's name
  $priorities['unassigned'] = 0;
  $priorities['low'] = 0;
  $priorities['med'] = 0;
  $priorities['high'] = 0;

  if (count($lead_ids) > 0) {
    $leads = db_select('leads', 'l')
      ->fields('l')
      ->condition('lead_id', $lead_ids, 'IN')
      ->condition('status', 'UNCOMMITTED', '=')
      ->orderBy('priority', 'ASC')
      ->execute();

    if ($leads->rowCount() > 0) {
      while ($lead = $leads->fetchAssoc()) {
        $leads_list .= '<li data-lead-id="' . $lead['lead_id'] . '" data-assigned-rep-id="' . $sales_rep_user_id . '" class="' . (($lead['status'] !== 'COMMITTED') ? 'uncommitted-lead ' : '') . 'list-group-item sales-rep-lead" data-url="' . base_path() . 'acrocrm_leads/">';

        $leads_list .= '<div class="pull-right text-right">';

        if ($lead['status'] !== 'COMMITTED') {
          $leads_list .= '<div class="btn-group" role="group" aria-label="...">';
          $leads_list .= '<button data-tooltip="tooltip" title="' . t('Edit Lead') . '" type="button" class="edit-lead-button btn btn-xs btn-primary" data-url="' . base_path() . 'acrocrm_leads/edit_lead/' . $lead['lead_id'] . '" data-toggle="modal" data-target="#lead-edit-modal"><span class="glyphicon glyphicon-pencil"></span></button>';
          $leads_list .= '<button data-tooltip="tooltip" title="' . t('Delete Lead') . '" type="button" class="delete-lead-button btn btn-xs btn-danger" data-url="' . base_path() . 'acrocrm_leads/delete_lead/' . $lead['lead_id'] . '" data-toggle="modal" data-target="#delete-lead-confirmation-modal"><span class="glyphicon glyphicon-trash"></span></button>';
          $leads_list .= '</div>';
        }
        else {
          $leads_list .= '<span data-tooltip="tooltip" title="' . t('Committed') . '" class="ball-indicator green-ball"></span>';
        }

        $leads_list .= '</div>';
        if (strlen($lead['firstname']) > 0) {
          $leads_list .= '<h5>' . $lead['firstname'] . ' ' . $lead['name'];
        }
        else {
          $leads_list .= '<h5><span class="accordion-title">' . $lead['name'];
        }
        $leads_list .= '</h5>';

        // start accordion div
        $leads_list .= '<div>';

        if (isset($lead['email']) && $lead['email'] != '') {
          $leads_list .= '<p class="list-group-item-text"><strong>' . t('Email') . ': </strong>' . $lead['email'] . '</p>';
        }

        if (isset($lead['phone']) && $lead['phone'] != '') {
          $leads_list .= '<p class="list-group-item-text"><strong>' . t('Phone') . ': </strong>' . $lead['phone'] . '</p>';
        }

        // determine what the priority of the lead is so we can show it in the rep div
        $priority = $lead['priority'];
        if ($priority == '') {
          $priorities['unassigned'] += 1;
        }
        else {
          if ($priority == 'low') {
            $priorities['low'] += 1;
          }
          else {
            if ($priority == 'medium') {
              $priorities['med'] += 1;
            }
            else {
              if ($priority == 'high') {
                $priorities['high'] += 1;
              }
            }
          }
        }

        // TODO: Make priority dropdown into 4 styled radio buttons instead
        $leads_list .= '<p class="list-group-item-text"><strong>' . t('Priority') . ': </strong>';
        if ($priority === '') {
          $priority = 'unassigned';
        }
        $leads_list .= $priority . '</p>';

        if (isset($lead['comments']) && $lead['comments'] != '') {
          $leads_list .= '<p class="list-group-item-text readmore"><strong>' . t('Comments') . ': </strong>' . $lead['comments'] . '</p>';
        }
        $leads_list .= '</div>';

        $leads_list .= '</li>';
      }
    }
    else {
      $leads_list .= '<li class="list-group-item no-assigned-leads">' . t('No Assigned Leads') . '</li>';
    }
  }
  else {
    $leads_list .= '<li class="list-group-item no-assigned-leads">' . t('No Assigned Leads') . '</li>';
  }

  return $priorities;
}

function acrocrm_leads_new_lead_ajax() {
  $form = drupal_get_form('acrocrm_leads_new_lead_form');

  $form["#prefix"] =
    '<div class="modal-header">
        <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
        <h4 class="modal-title" id="editLead">' . t('Create New Lead') . '</h4>
      </div>
      <div class="modal-body">';

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create Lead'),
    '#prefix' => '</div><div class="modal-footer"><button type="button" class="btn btn-default" data-dismiss="modal">' . t('Close') . '</button>',
    '#suffix' => '</div>',
    '#weight' => 7,
    '#attributes' => array(
      'class' => array('btn', 'btn-primary'),
    ),
  );

  echo drupal_render($form);
}

function acrocrm_leads_edit_lead_form($form, &$form_state) {
  generate_lead_form($form, $form_state);
  return $form;
}

function acrocrm_leads_new_lead_form($form, &$form_state) {
  generate_lead_form($form, $form_state);
  return $form;
}

function generate_lead_form(&$form, &$form_state) {
  $form['priority'] = array(
    '#type' => 'select',
    '#title' => t('Priority'),
    '#options' => array(
      'high' => t('High'),
      'medium' => t('Medium'),
      'low' => t('Low'),
    ),
    '#required' => FALSE,
    '#weight' => 0,
    '#empty_value' => '',
    '#description' => t('The priority of the lead.'),
    '#prefix' => '<div class="row"><div class="col-lg-6 col-md-6"><div class="form-group">',
    '#suffix' => '</div></div>',
    '#attributes' => array(
      'class' => array('form-control'),
    ),
  );

  /* Create list of users who are sales representatives. */
  $users = entity_load('user');
  $sales_reps = array();

  foreach ($users as $user) {
    if (user_access('sales rep', $user)) {
      $sales_reps[$user->uid] = $user->name;
    }
  }

  $form['assign'] = array(
    '#type' => 'select',
    '#title' => t('Assign to'),
    '#options' => $sales_reps,
    '#required' => FALSE,
    '#weight' => 1,
    '#empty_value' => '',
    '#description' => t('The sales rep which the lead should be assigned to.'),
    '#prefix' => '<div class="col-lg-6 col-md-6"><div class="form-group">',
    '#suffix' => '</div></div></div>',
    '#attributes' => array(
      'class' => array('form-control'),
    ),
  );

  $form['firstname'] = array(
    '#type' => 'textfield',
    '#title' => t('First Name'),
    '#size' => 50,
    '#maxlength' => 50,
    '#required' => FALSE,
    '#weight' => 2,
    '#prefix' => '<div class="row"><div class="col-lg-6"><div class="form-group">',
    '#suffix' => '</div></div>',
    '#attributes' => array(
      'class' => array('form-control'),
    ),
  );

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Last Name'),
    '#size' => 50,
    '#maxlength' => 50,
    '#required' => TRUE,
    '#weight' => 3,
    '#prefix' => '<div class="col-lg-6"><div class="form-group">',
    '#suffix' => '</div></div></div>',
    '#attributes' => array(
      'class' => array('form-control'),
    ),
  );

  $form['email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email'),
    '#size' => 50,
    '#maxlength' => 50,
    '#required' => TRUE,
    '#weight' => 4,
    '#prefix' => '<div class="row"><div class="col-lg-6"><div class="form-group">',
    '#suffix' => '</div></div>',
    '#attributes' => array(
      'class' => array('form-control'),
    ),
  );

  $form['phone'] = array(
    '#type' => 'textfield',
    '#title' => t('Phone Number'),
    '#size' => 32,
    '#maxlength' => 15,
    '#required' => FALSE,
    '#weight' => 5,
    '#prefix' => '<div class="col-lg-6"><div class="form-group">',
    '#suffix' => '</div></div></div>',
    '#attributes' => array(
      'class' => array('form-control'),
    ),
  );

  $form['comments'] = array(
    '#type' => 'textarea',
    '#title' => t('Comments'),
    '#maxlength' => 1000,
    '#required' => TRUE,
    '#weight' => 6,
    '#prefix' => '<div class="row"><div class="col-lg-12"><div class="form-group">',
    '#suffix' => '</div></div></div>',
    '#attributes' => array(
      'class' => array('form-control'),
    ),
  );

  $form['lead_id'] = array(
    '#type' => 'hidden',
  );

  $form['redirect_page'] = array(
    '#type' => 'hidden',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
    '#prefix' => '<div class="row"><div class="col-lg-12"><div class="form-group">',
    '#suffix' => '</div></div></div>',
    '#weight' => 7,
    '#attributes' => array(
      'class' => array('btn', 'btn-primary'),
    ),
  );
}

/**
 * Submit callback for acrocrm_leads_edit_lead_form().
 */
function acrocrm_leads_edit_lead_form_submit($form, $form_state) {
  db_update('leads')
    ->fields(array(
      'name' => $form_state['values']['name'],
      'firstname' => $form_state['values']['firstname'],
      'email' => $form_state['values']['email'],
      'phone' => $form_state['values']['phone'],
      'comments' => $form_state['values']['comments'],
      'priority' => $form_state['values']['priority']
    ))
    ->condition('lead_id', $form_state['values']['lead_id'], '=')
    ->execute();

  $check_exists = db_select('assigned_leads', 'al')
    ->fields('al')
    ->condition('lead_id', $form_state['values']['lead_id'], '=')
    ->execute();

  if ($form_state['values']['assign'] != 0) {
    if ($check_exists->rowCount() > 0) {
      db_update('assigned_leads')
        ->fields(array(
          'uid' => $form_state['values']['assign']
        ))
        ->condition('lead_id', $form_state['values']['lead_id'], '=')
        ->execute();
    }
    else {
      db_insert('assigned_leads')
        ->fields(array(
          'uid' => $form_state['values']['assign'],
          'lead_id' => $form_state['values']['lead_id']
        ))
        ->execute();
    }
  }
  else {
    if ($check_exists->rowCount() > 0) {
      db_delete('assigned_leads')
        ->condition('lead_id', $form_state['values']['lead_id'])
        ->execute();
    }
  }

  drupal_set_message(t('The lead has been updated successfully.'));
}

/**
 * Submit callback for acrocrm_leads_new_lead_form().
 */
function acrocrm_leads_new_lead_form_submit($form, $form_state) {
  $lead_id = db_insert('leads')
    ->fields(array(
      'name' => $form_state['values']['name'],
      'firstname' => $form_state['values']['firstname'],
      'email' => $form_state['values']['email'],
      'phone' => $form_state['values']['phone'],
      'comments' => $form_state['values']['comments'],
      'priority' => $form_state['values']['priority']
    ))
    ->execute();

  if ($form_state['values']['assign'] != '') {
    db_insert('assigned_leads')
      ->fields(array(
        'uid' => $form_state['values']['assign'],
        'lead_id' => $lead_id,
      ))
      ->execute();
  }

  drupal_set_message(t('The new lead has been created successfully.'));
}

/**
 * Implements validation from the Form API for the new lead form.
 *
 * @param $form
 *   A structured array containing the elements and properties of the form.
 * @param $form_state
 *   An array that stores information about the form's current state
 *   during processing.
 */
function acrocrm_leads_new_lead_form_validate($form, &$form_state) {
  generate_lead_validation($form, $form_state);
}

/**
 * Implements validation from the Form API for the edit lead form.
 *
 * @param $form
 *   A structured array containing the elements and properties of the form.
 * @param $form_state
 *   An array that stores information about the form's current state
 *   during processing.
 */
function acrocrm_leads_edit_lead_form_validate($form, &$form_state) {
  generate_lead_validation($form, $form_state);
}

function generate_lead_validation(&$form, &$form_state) {
  $email = $form_state['values']['email'];
  $email_pattern = '/^[^@]*@[^@]*/';
  if ($email != '' && !preg_match($email_pattern, $email)) {
    form_set_error('contact_email', t('You must enter a valid email address.'));
  }
}
