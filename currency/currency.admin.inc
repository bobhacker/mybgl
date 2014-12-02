<?php
/**
 * @file currency.admin.inc
 */
// AED,AUD, CHF,CNY, EUR,GBP, HDK,NOK, RUB, SEK

//variable_del('currency_allowed');
//variable_del('currency_default');
/**
 * Currency settings form.
 */
function currency_settings_form($form, &$form_state) {
  $form = array();
  $currencies = variable_get('currency_allowed', array(CURRENCY_DEFAULT));
  $form['currency_allowed'] = array(
    '#type' => 'textfield',
    '#title' => t('Allowed currencies:'),
    '#description' => t('Enter one or more currencies : %example.', array('%example' => 'value1, value2, value3')),
    '#default_value' => implode(', ', $currencies),
    '#required' => TRUE,
  );
  $form['currency_default'] = array(
    '#type' => 'select',
    '#title' => t('Default currency:'),
    '#description' => t('Select the default currency.'),
    '#options' => array_combine($currencies, $currencies),
    '#default_value' => variable_get('currency_default', CURRENCY_DEFAULT),
    '#required' => TRUE,
  );
  $form['#validate'] = array('currency_settings_form_validate');
  return system_settings_form($form);
}

function currency_settings_form_validate($form, &$form_state) {
  $currencies = explode(',', $form_state['values']['currency_allowed']);
  $currencies = array_map('trim', $currencies);          // Trim all array elements
  $currencies = array_map('mb_strtoupper', $currencies); // Puts array elements in uppercase (UTF-8 safe)
  $currencies = array_filter($currencies, 'strlen');     // Only returns unempty array elements
  sort($currencies, SORT_STRING);                        // Sort array elements alphabetically
  $form_state['values']['currency_allowed'] = $currencies;
}
