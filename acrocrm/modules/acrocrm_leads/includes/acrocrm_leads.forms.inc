<?php
/**
 * @file
 * Contains the add lead page form and the block contact form settings form.
 */

/**
 * Page callback:  settings
 *
 * @see form_test_menu()
 */
function acrocrm_leads_contact_form_settings_form($form, &$form_state) {
  // @TODO: system variables should always prefix with module name
  $form['name_field'] = array(
    '#type' => 'radios',
    '#title' => t('Name field type'),
    '#default_value' => variable_get('name_field', 'name'),
    '#options' => array('name' => 'Just a Name field', 'firstname_lastname' => 'Firstname/Lastname'),
    '#description' => t('Should the form contain a single name field, or a firstname and a lastname field?')
  );

  return system_settings_form($form);
}
