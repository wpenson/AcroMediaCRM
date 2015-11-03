<div id="wrapper">
  <!-- Navigation -->
  <nav class="navbar navbar-default navbar-static-top" role="navigation" style="margin-bottom: 0">
    <div class="navbar-header">
      <a class="navbar-brand" href="index.html">SB Admin v2.0</a>
    </div>
    <!-- /.navbar-header -->
    <div class="navbar-default sidebar" role="navigation">
      <div class="sidebar-nav navbar-collapse">
        <?php if ($page['sidebar_first']): ?>
          <ul class="nav" id="side-menu">
            <?php print render($page['sidebar_first']); ?>
          </ul>
        <?php else: ?>
          <p>There are no links.</p>
        <?php endif; ?>
      </div>
      <!-- /.sidebar-collapse -->
    </div>
    <!-- /.navbar-static-side -->
  </nav>
  <div id="page-wrapper">
    <?php print render($page['content']); ?>
  </div>
  <!-- /#page-wrapper -->
</div>
