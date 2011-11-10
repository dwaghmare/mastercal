(function($){
  $(function() { 
    $(".more-event").hide();
    $(".event-show").click(function() {
      $(this).siblings('.more-event').toggle();
    });
  })
 
})(jQuery);