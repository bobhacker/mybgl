<?php
/**
 * @file bg_re_import_user.admin.inc
 */

/**
 * Opt-in form.
 */
function bg_re_import_user_optin_form($form, $form_state) {
  $form = array();
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'E-mailing',
    '#submit' => array('bg_re_import_user_optin_form_submit'),
  );
  return $form;
}

/**
 * Submit callback.
 */
function bg_re_import_user_optin_form_submit($form, &$form_state) {
  $query = db_select('users', 'u')
    ->fields('u', array('mail'))
    ->condition('u.uid', 1, '>');
  $emails = $query->execute()->fetchCol();
  foreach ($emails as $email) {
    $module = 'bg_re_import_user';
    $key = 'optin';
    $to = $email;
    $language = language_default();
    $params = array(
      'landing_page' => url('bgl/re-import-user/optin', array('absolute' => TRUE, 'query' => array(
        'email' => $email, 'hash' => hash('sha256', $email)
      )))
    );
    $from = NULL;
    $send = TRUE;
    $message = drupal_mail($module, $key, $to, $language, $params, $from, $send);
    if (isset($message['result']) && $message['result']) {
      drupal_set_message(format_string("E-mail '%sujet' envoyé avec succès vers %destinataire.", array(
        '%sujet' => $message['subject'],
        '%destinataire' => $to,
      )));
    }
  }
}