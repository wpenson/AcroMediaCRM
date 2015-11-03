<?php

function acrocrm_leads_overview_page() {

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