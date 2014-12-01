<?php
/**
 * @file bg_bebritish.admin.inc
 */

/**
 * Landing page des inscrits BeBritish. Vérification du lien + édition du mot de passe.
 * @see bg_bebritish_menu()
 * @see user_pass_reset()
 */
function bg_bebritish_password_form($form, &$form_state, $uid, $timestamp, $hashed_pass, $action = NULL) {
  global $user;

  // When processing the one-time login link, we have to make sure that a user
  // isn't already logged in.
  if ($user->uid) {
    // The existing user is already logged in.
    if ($user->uid == $uid) {
      drupal_set_message(t('You are logged in as %user. <a href="!user_edit">Change your password.</a>', array('%user' => $user->name, '!user_edit' => url("user/$user->uid/edit"))));
    }
    // A different user is already logged in on the computer.
    else {
      $reset_link_account = user_load($uid);
      if (!empty($reset_link_account)) {
        drupal_set_message(t('Another user (%other_user) is already logged into the site on this computer, but you tried to use a one-time link for user %resetting_user. Please <a href="!logout">logout</a> and try using the link again.',
        array('%other_user' => $user->name, '%resetting_user' => $reset_link_account->name, '!logout' => url('user/logout'))));
      } else {
        // Invalid one-time link specifies an unknown user.
        drupal_set_message(t('The one-time login link you clicked is invalid.'));
      }
    }
    drupal_goto();
  }
  else {
    // Time out, in seconds, until login URL expires. Defaults to 24 hours =
    // 86400 seconds.
    $timeout = variable_get('user_password_reset_timeout', 86400);
    $current = REQUEST_TIME;
    // Some redundant checks for extra security ?
    $users = user_load_multiple(array($uid));
    if ($timestamp <= $current && $account = reset($users)) {
      // Activation manuelle du compte utilisateur sinon le système bug, car la
      // demande de nouveau mot de passe est interdite pour les comptes bloqués
      user_save($account, array('status' => '1'));
      // Au passage, mise à jour du is_newsletter_bgl à 1. Parce que
      db_update('bg_customers')
        ->fields(array('is_newsletter_bgl' => 1, 'created' => REQUEST_TIME))
        ->condition('uid', $user->uid)
        ->execute();
      
      // No time out for first time login.
      if ($account->login && $current - $timestamp > $timeout) {
        drupal_set_message(t('You have tried to use a one-time login link that has expired. Please request a new one using the form below.'));
        drupal_goto('user/password');
      }
      elseif ($account->uid && $timestamp >= $account->login && $timestamp <= $current && $hashed_pass == user_pass_rehash($account->pass, $timestamp, $account->login)) {
        // First stage is a confirmation form, then login
        if ($action == 'login') {
          // Set the new user.
          $user = $account;
          // user_login_finalize() also updates the login timestamp of the
          // user, which invalidates further use of the one-time login link.
          user_login_finalize();
          watchdog('user', 'User %name used one-time login link at time %timestamp.', array('%name' => $account->name, '%timestamp' => $timestamp));
          drupal_set_message(t('You have just used your one-time login link. It is no longer necessary to use this link to log in. Please change your password.'));
          // Let the user's password be changed without the current password check.
          $token = drupal_random_key();
          $_SESSION['pass_reset_' . $user->uid] = $token;
          drupal_goto('user/' . $user->uid . '/edit', array('query' => array('pass-reset-token' => $token)));
        }
        else {
          $form['message'] = array('#markup' => t('<p>This is a one-time login for %user_name and will expire on %expiration_date.</p><p>Click on this button to log in to the site and change your password.</p>', array('%user_name' => $account->name, '%expiration_date' => format_date($timestamp + $timeout))));
          $form['help'] = array('#markup' => '<p>' . t('This login can be used only once.') . '</p>');
          $form['actions'] = array('#type' => 'actions');
          $form['actions']['submit'] = array('#type' => 'submit', '#value' => t('Log in'));
          $form['#action'] = url("bgl/bebritish/optin/$uid/$timestamp/$hashed_pass/login");
          return $form;
        }
      }
      else {
        drupal_set_message(t('You have tried to use a one-time login link that has either been used or is no longer valid. Please request a new one using the form below.'));
        drupal_goto('user/password');
      }
    }
    else {
      // Deny access, no more clues.
      // Everything will be in the watchdog's URL for the administrator to check.
      drupal_access_denied();
      drupal_exit();
    }
  }
}
