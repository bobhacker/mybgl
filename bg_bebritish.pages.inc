<?php
/**
 * @file bg_bebritish.pages.inc
 */

/**
 * Formulaire d'importation de la table {mkt_bbritish_users}.
 */
function bg_bebritish_import_form($form, &$form_state) {
  $form = array();
  $form['help'] = array(
    '#type' => 'item',
    '#markup' => 'Import de la table <em>mkt_bbritish_users</em> :',
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Importer',
    '#submit' => array('bg_bebritish_import_form_submit'),
  );
  return $form;
}

/**
 * Importation de la table {mkt_bbritish_users}.
 */
function bg_bebritish_import_form_submit() {
  set_time_limit(0);
  $transaction = db_transaction();
  try {
    $query = db_select('mkt_bbritish_users', 'm')
      ->fields('m', array('email', 'nom', 'prenom', 'cp', 'telephone'));
    $results = $query->execute();
    foreach ($results as $row) {
      $account = user_load_by_mail($row->email);
      // Uniquement si non inscrit
      if (!$account) {
        $name = strtolower(substr($row->prenom, 0, 1) . '.' . $row->nom);
        $cpt = 0;
        // On cherche un name unique
        while (user_load_by_name($name)) {
          $name = strtolower(substr($row->prenom, 0, 1) . '.' . $row->nom) . "_$cpt";
          $cpt++;
        }
        // Informations à insérer
        $edit = array(
          'name' => $name,
          'pass' => '',
          'mail' => $row->email,
          'created' => REQUEST_TIME,
          'status' => 0,
        );
        // Insertion + mise à jour de la table {bg_customers}
        if (user_save(NULL, $edit)) {
          db_update('bg_customers')
            ->fields(array(
              'first_name' => $row->nom,
              'last_name' => $row->prenom,
              'zip_code' => $row->cp,
              'phone_number' => $row->telephone,
              'media_subscribe' => 'BeBritish-borne',
            ))
            ->condition('email', $row->email)
            ->execute();
        }
        else {
          drupal_set_message(format_string("Erreur à l'import de l'utilisateur %utilisateur.", array(
            '%utilisateur' => $row->email
          )));
        }
      }
    }
    drupal_set_message('Imports effectués.');
  }
  catch (Exception $e) {
    $transaction->rollback();
    watchdog_exception(__FUNCTION__, $e);
    throw $e;
  }
}

/**
 * Formulaire d'e-mailing opt-in pour les membres BeBritish.
 */
function bg_bebritish_emailing_form($form, $form_state) {
  $form = array();
  
  // Liste paginée des inscrits BeBritish
  $query = db_select('bg_customers', 'c')
    ->fields('u', array('uid', 'name', 'pass', 'mail', 'login'))
    ->fields('c', array('first_name', 'last_name'))
    ->condition('c.is_newsletter_bgl', 0)
    ->condition('c.media_subscribe', 'bebritish-%', 'LIKE');
  $query->join('users', 'u', 'u.uid = c.uid');
  
  $s_query = serialize($query);
  $s_results = $query->execute();
  
  $query = $query->extend('PagerDefault')->limit(10);
  $results = $query->execute();
  
  $rows = array();
  $page = isset($_GET['page']) ? (int) $_GET['page'] : 0;
  $cpt = 1;
  foreach ($results as $row) {
    $compteur = ($cpt + $page * 10) <= $s_results->rowCount() ? $cpt + $page * 10 : $cpt;
    $rows[] = array($compteur, $row->mail, $row->first_name, $row->last_name, $row->name);
    $cpt++;
  }
  $form['destinataires'] = array(
    '#type' => 'item',
    '#title' => "Liste d'envoi ({$s_results->rowCount()})",
    '#theme' => 'table',
    '#header' => array('#', 'E-mail', 'Prénom', 'Nom', 'Nom d\'utilisateur'),
    '#rows' => $rows,
    '#suffix' => theme('pager', array('tags' => array())),
  );
  
  $form['query'] = array(
    '#type' => 'hidden',
    '#value' => $s_query,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'E-mailing',
    '#submit' => array('bg_bebritish_emailing_form_submit'),
  );
  
  return $form;
}

/**
 * E-mailing opt-in pour les membres BeBritish.
 * @see user_pass_reset_url()
 * @see drupal_mail()
 */
function bg_bebritish_emailing_form_submit($form, &$form_state) {
  $query = unserialize($form_state['values']['query']);
  $results = $query->execute();
  foreach ($results as $account) {
    $module = 'bg_bebritish';
    $key = 'optin-bebritish';
    $to = $account->mail;
    $language = language_default();
    $timestamp = REQUEST_TIME;
    $landing_page = url("bgl/bebritish/optin/$account->uid/$timestamp/" . user_pass_rehash($account->pass, $timestamp, $account->login), array('absolute' => TRUE));
    $params = array(
      'prenom' => $account->first_name,
      'nom' => $account->last_name,
      'landing_page' => $landing_page,
    );
    $from = NULL;
    $send = TRUE;
    /** Simulation d'envoi de l'email - Réactiver si nécessaire **/
    //$message = drupal_mail($module, $key, $to, $language, $params, $from, $send);
    //if (isset($message['result']) && $message['result']) {
    //  drupal_set_message(format_string("E-mail '%sujet' envoyé avec succès à l'utilisateur %utilisateur.", array(
    //    '%sujet' => $message['subject'],
    //    '%utilisateur' => $account->name,
    //  )));
    //}
  }
}
