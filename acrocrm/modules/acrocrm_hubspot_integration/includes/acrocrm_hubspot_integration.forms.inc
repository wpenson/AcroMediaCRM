<?php
function acrocrm_hubspot_integration_settings_form() {
  // @TODO: system variables should always prefix with module name
  $form['api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('HubSpot API key'),
    '#default_value' => variable_get('api_key'),
    '#description' => t('The HubSpot API key for the developer account used for testing.')
  );

  $form['automatic_creation'] = array(
    '#type' => 'checkbox',
    '#title' => t('Create leads on HubSpot automatically after they are assigned'),
    '#default_value' => variable_get('automatic_creation'),
    '#description' => t('This option allows leads to be created on HubSpot automatically without the sales rep logging in and sending each lead to HubSpot one at a time.')
  );

  return system_settings_form($form);
}
