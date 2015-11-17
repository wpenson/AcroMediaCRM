<?php

function acrocrm_leads_overview_page() {

    // load the leads from the database
    $leads = acrocrm_leads_load_leads();

    $page['content']['leads'] = array(
        '#markup' => acrocrm_leads_generate_leads_html($leads)
    );

    $page['content']['reps'] = array(
        '#markup' => acrocrm_leads_get_sales_reps()
    );

    return theme('acrocrm_leads_home', array('page' => $page));
}

/*
 * Get a list of all leads from the database
 */
function acrocrm_leads_load_leads() {
    $leads =  db_query('SELECT * from leads');

    $leads_arr = array();

    if ($leads) {

        foreach ($leads as $lead) {
            $leads_arr[] = array(
                'firstname' => $lead->firstname,
                'name' => $lead->name,
                'email' =>$lead->email,
                'phone' =>$lead->phone,
                'comments' =>$lead->comments,
                'priority' =>$lead->priority
            );
        }
    }
    else {
        $leads_arr[] = 'An error occurred, could not retrieve leads from database.';
    }

    return $leads_arr;
}

/*
 * Create the HTML to display a lead
 */
function acrocrm_leads_generate_leads_html($leads) {
    $search_div = '<div class="input-group">
                <input type="text" class="form-control" aria-label="..." placeholder="Search leads">
                <div class="input-group-btn">
                  <button type="button" class="btn btn-default">Action</button>
                  <button type="button" class="btn btn-default dropdown-toggle" data-toggle="dropdown" aria-haspopup="true" aria-expanded="false">
                    <span class="caret"></span>
                    <span class="sr-only">Toggle Dropdown</span>
                  </button>
                  <ul class="dropdown-menu dropdown-menu-right">
                    <li><a href="#">Show all</a></li>
                    <li><a href="#">Another action</a></li>
                    <li><a href="#">Something else here</a></li>
                    <li role="separator" class="divider"></li>
                    <li><a href="#">Separated link</a></li>
                  </ul>
                </div>
              </div>';

    $leads_list = '';
    foreach ($leads as $lead) {
        $leads_list .= '<a class="list-group-item lead-draggable" href="#">';

        if (strlen($lead['firstname']) > 0) {
            $leads_list .= '<h4 class="list-group-item-heading">' . $lead['firstname'] . ' ' . $lead['name'] . '</h4>';
        }
        else {
            $leads_list .= '<h4 class="list-group-item-heading">' . $lead['name'] . '</h4>';
        }

        $leads_list .= '<div class="priority pull-right">Priority:
            <select id="set-priority" class="input-group">
                <option value="unassigned">unassigned</option>
                <option value="low">low</option>
                <option value="medium">medium</option>
                <option value="high">high</option>
            </select></div>';

        $leads_list .= '<p class="list-group-item-text"><strong>Email: </strong>' . $lead['email'] . '</p>';
        $leads_list .= '<p class="list-group-item-text"><strong>Phone: </strong>' . $lead['phone'] . '</p>';
        $leads_list .= '<p class="list-group-item-text"><strong>Comments: </strong>' . $lead['comments'] . '</p>';

        $leads_list .=  '</a>';
    }

    $leads_div = '<div id="leads-list" class="list-group col-xs-6 col-lg-4">' . $search_div . $leads_list . '</div>';

    return $leads_div;
}

/*
 * Get a list of all sales reps and create html to display sales reps
 */
function acrocrm_leads_get_sales_reps() {
    $users = entity_load('user');

    $rep_div = '';

    foreach ($users as $user) {
        if (user_access('sales rep', $user)) {
            $rep_div = $rep_div . '<div id="' . $user->uid . '" class="lead-rep-overview">' . $user->name . '</div>';
        }
    }

    $sales_reps = '<div class="col-md-6">' . $rep_div . '</div>';

    return $sales_reps;
}

function acrocrm_leads_new_lead_page() {
  $page['content']['new_lead_form'] = drupal_get_form('acrocrm_leads_new_lead_form');

  return theme('acrocrm_leads_home', array('page' => $page));
}

function acrocrm_leads_new_lead_form($form, &$form_state) {
  $form['priority'] = array(
    '#type' => 'select',
    '#title' => t('Priority'),
    '#options' => array(
      'high' => t('High'),
      'medium' => t('Medium'),
      'low' => t('Low'),
    ),
    '#required' => FALSE,
    '#empty_value' => '',
    '#description' => t('The priority of the lead.'),
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
    '#empty_value' => '',
    '#description' => t('The sales rep which the lead should be assigned to.'),
  );

  $form['firstname'] = array(
    '#type' => 'textfield',
    '#title' => t('First Name'),
    '#size' => 50,
    '#maxlength' => 50,
    '#required' => FALSE,
  );


  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Last Name'),
    '#size' => 50,
    '#maxlength' => 50,
    '#required' => TRUE
  );


  $form['email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email'),
    '#size' => 50,
    '#maxlength' => 50,
    '#required' => TRUE
  );

  $form['phone'] = array(
    '#type' => 'textfield',
    '#title' => t('Phone Number'),
    '#size' => 32,
    '#maxlength' => 15,
    '#required' => FALSE
  );

  $form['comments'] = array(
    '#type' => 'textarea',
    '#title' => t('Comments'),
    '#maxlength' => 1000,
    '#required' => TRUE
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
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
               'priority' => $form_state['values']['priority']))
    ->execute();

  if ($form_state['values']['assign'] != '') {
    db_insert('assigned_leads')
      ->fields(array(
                 'uid' => $form_state['values']['assign'],
                 'lead_id' => $lead_id,
               ))
      ->execute();
  }

  drupal_set_message(t('The lead has been created successfully.'));
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
  $email = $form_state['values']['email'];
  $email_pattern = '/^[^@]*@[^@]*/';
  if ($email != '' && !preg_match($email_pattern, $email)) {
    form_set_error('contact_email', t('You must enter a valid email address.'));
  }
}