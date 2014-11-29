<?php
/**
 * @file bg_bebritish.admin.inc
 */

/**
 * Opt-in form.
 */
function bg_bebritish_settings_form($form, $form_state) {
  $form = array();
  
  $query = db_select('users', 'u')
    ->fields('u', array('uid', 'name', 'pass', 'mail', 'login'))
    ->condition('u.uid', 1, '>');
  $results = $query->execute();
  $rows = array();
  $cpt = 1;
  foreach ($results as $row) {
    $rows[] = array($cpt, $row->name, $row->mail);
    $cpt++;
  }
  $form['destinataires'] = array(
    '#type' => 'item',
    '#title' => "Liste d'envoi ({$results->rowCount()})",
    '#theme' => 'table',
    //'#caption' => 'Liste d\'envoi',
    '#header' => array('#', 'Nom', 'E-mail'),
    '#rows' => $rows,
  );
  $form['query'] = array(
    '#type' => 'hidden',
    '#value' => serialize($query),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'E-mailing',
    '#submit' => array('bg_bebritish_settings_form_submit'),
  );
  return $form;
}

/**
 * Submit callback.
 * @see drupal_mail()
 * @see user_pass_reset_url()
 */
function bg_bebritish_settings_form_submit($form, &$form_state) {
  $query = unserialize($form_state['values']['query']);
  $results = $query->execute();
  foreach ($results as $account) {
    $module = 'bg_bebritish';
    $key = 'optin';
    $to = $account->mail;
    $language = language_default();
    $timestamp = REQUEST_TIME;
    $landing_page = url("bgl/bebritish/optin/$account->uid/$timestamp/" . user_pass_rehash($account->pass, $timestamp, $account->login), array('absolute' => TRUE));
    $params = array(
      'nom' => $account->name,
      'landing_page' => $landing_page,
    );
    $from = NULL;
    $send = TRUE;
    $message = drupal_mail($module, $key, $to, $language, $params, $from, $send);
    if (isset($message['result']) && $message['result']) {
      drupal_set_message(format_string("E-mail '%sujet' envoyé avec succès à l'utilisateur %utilisateur.", array(
        '%sujet' => $message['subject'],
        '%utilisateur' => $account->name,
        '@destinataire' => $to,
      )));
    }
  }
}
