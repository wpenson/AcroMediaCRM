<div id="wrapper">
  <!-- Navigation -->
  <nav class="navbar navbar-default navbar-static-top" role="navigation" style="margin-bottom: 0">
    <div class="navbar-header">
      <a class="navbar-brand" href="<?php print drupal_get_path('page', 'acrocrm') ?>">Acro Media CRM</a>
    </div>
    <!-- /.navbar-header -->
    <h1>Sidebar</h1>
    <div class="navbar-default sidebar" role="navigation">
      <div class="sidebar-nav navbar-collapse">
        <?php if ($page['sidebar_first']): ?>
          <ul class="nav" id="side-menu">
            <?php print render($page['sidebar_first']); ?>
          </ul>
        <?php else: ?>
          <p>There are no modules installed.</p>
        <?php endif; ?>
      </div>
      <!-- /.sidebar-collapse -->
    </div>
    <!-- /.navbar-static-side -->
  </nav>
  <div id="page-wrapper">
    <?php if ($messages): ?>
      <div id="console" class="clearfix"><?php print $messages; ?></div>
    <?php endif; ?>
    <h1>Content</h1>
    <?php print render($page['content']); ?>
  </div>
  <!-- /#page-wrapper -->
</div>
