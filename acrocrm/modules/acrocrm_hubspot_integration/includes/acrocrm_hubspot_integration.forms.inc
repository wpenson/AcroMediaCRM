<?php
function acrocrm_hubspot_integration_settings_form() {
    $form['api_key'] = array(
        '#type' => 'textfield',
        '#title' => t('HubSpot API key'),
        '#default_value' => variable_get('api_key'),
        '#description' => t('The HubSpot API key for the developer account used for testing.')
    );

    return system_settings_form($form);
}