<?php
function acrocrm_hubspot_integration_settings_form() {
  $form['acrocrm_hubspot_integration_hub_id'] = array(
    '#type' => 'textfield',
    '#title' => t('HubSpot Hub ID'),
    '#default_value' => variable_get('acrocrm_hubspot_integration_hub_id'),
    '#description' => t('The HubSpot Hub ID of the account that you want to integrate with.')
  );

  $form['automatic_creation'] = array(
    '#type' => 'checkbox',
    '#title' => t('Create leads on HubSpot automatically after they are assigned'),
    '#default_value' => variable_get('automatic_creation'),
    '#description' => t('This option allows leads to be created on HubSpot automatically without the sales rep logging in and sending each lead to HubSpot one at a time.')
  );

  return system_settings_form($form);
}
