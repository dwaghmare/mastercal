<?php

/**
  * Template for whitepages block
  *
  * Available variables:
  * - $variables - all available variables
  * - $list:
  *    -  'fname' => first name 
          'lname' => last name
          'href' => link to phonebook page
          'title' => position
          'phone' => office phone number or ''
          'mail' => email address
          'office' => office address
          'dept' => department name
    - Renderables: 
        - $emp_table
          - use: print render($emp_table)
          - returns a rendered table with headers('Name', 'Title', 'ITS group', 'Address', 'Phone')
        - $emp_ul
          - renders the fname, lname linked to the whitepage in a <ul>
  */

?>

 
<div class="whitepage_wrap">
  <?php print render($emp_table); ?>
</div>

