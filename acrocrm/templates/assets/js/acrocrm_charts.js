//Flot Pie Chart
$(function() {
  var total_leads = $('#leads-total').data('leadTotal');
  var data = [];
  $('.chart-data').each(function() {
    data.push({
      label: $(this).data('repname'),
      data: $(this).data('repcount') / total_leads
    })
  });

  var plotObj = $.plot($("#flot-pie-chart"), data, {
    series: {
      pie: {
        show: true
      }
    },
    grid: {
      hoverable: true
    },
    tooltip: true,
    tooltipOpts: {
      content: "%p.0%, %s", // show percentages, rounding to 2 decimal places
      shifts: {
        x: 20,
        y: 0
      },
      defaultTheme: true
    }
  });

  $("#flot-pie-chart div.legend > div").css({
    display: 'none'
  });

  $("#flot-pie-chart div.legend table").css({
    width: '100px'
  });
});
