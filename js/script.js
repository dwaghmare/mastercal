(function($){
  $(function() { 
    $(".more-event").hide();
    $(".event-show").click(function(ev) {
      ev.preventDefault();
      $(this).siblings('.more-event').toggle();
    });
  })
 
})(jQuery);