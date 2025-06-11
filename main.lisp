(in-package :flying-tank)

(defparameter *grid-size* 30)
(defparameter *cell-size* 50)
(defparameter *window-cols* 10)
(defparameter *window-rows* 10)
(defparameter *window-width* (* *cell-size* *window-cols*))
(defparameter *window-height* (* *cell-size* *window-rows*))

(defparameter *camera-x* 0)
(defparameter *camera-y* 0)

(defparameter *tank-sprite* nil)

;; Safe number validation
(defun safe-number-p (n)
  "Check if a number is finite and not NaN"
  (and (numberp n)
       (handler-case
           (progn
             ;; Test for NaN: NaN is not equal to itself
             (when (and (floatp n) (not (= n n)))
               (return-from safe-number-p nil))
             ;; Test for infinity by checking if it's larger than most-positive-float
             (when (and (floatp n) 
                        (or (> (abs n) most-positive-single-float)
                            (> (abs n) most-positive-double-float)))
               (return-from safe-number-p nil))
             ;; Additional check: ensure basic arithmetic works
             (let ((test (+ n 0)))
               (declare (ignore test))
               t))
         (error () nil))))

(defun safe-clamp (value min-val max-val)
  "Safely clamp a value between min and max"
  (cond
    ((not (safe-number-p value)) min-val)
    ((< value min-val) min-val)
    ((> value max-val) max-val)
    (t value)))

(defun safe-floor (n)
  "Safely floor a number, returning 0 for invalid inputs"
  (if (safe-number-p n)
      (floor n)
      0))

(defun load-texture (renderer path)
  (format t "~&=== TEXTURE LOADING DEBUG ===~%")
  (format t "Attempting to load texture from: ~a~%" path)
  (format t "Current working directory: ~a~%" (truename "."))
  (format t "File exists? ~a~%" (probe-file path))
  
  (handler-case
      (let ((surface (sdl2-image:load-image path)))
        (if surface
            (progn
              (format t "✓ Surface loaded successfully: ~a~%" surface)
              (let ((w (sdl2:surface-width surface))
                    (h (sdl2:surface-height surface)))
                (format t "Surface dimensions: ~ax~a~%" w h))
              (let ((texture (sdl2:create-texture-from-surface renderer surface)))
                (sdl2:free-surface surface)
                (if texture
                    (progn
                      (format t "✓ Texture created successfully: ~a~%" texture)
                      (let ((w (sdl2:texture-width texture))
                            (h (sdl2:texture-height texture)))
                        (format t "Texture dimensions: ~ax~a~%" w h))
                      texture)
                    (progn
                      (format t "✗ Failed to create texture from surface~%")
                      nil))))
            (progn
              (format t "✗ Failed to load surface for image: ~a~%" path)
              nil)))
    (error (e)
      (format t "✗ Exception while loading texture: ~a~%" e)
      nil)))

(defparameter *sprite-size-safe* 256)

(defun get-sprite-rect (direction)
  "Get sprite rectangle for a given direction with safety bounds checking"
  (let ((size *sprite-size-safe*))
    (handler-case
        (case direction
          (:north (sdl2:make-rect 0 0 size size))
          (:east  (sdl2:make-rect size 0 size size))
          (:south (sdl2:make-rect 0 size size size))
          (:west  (sdl2:make-rect size size size size))
          (t (sdl2:make-rect 0 0 size size)))
      (error (e)
        (format t "Error creating sprite rect for ~a: ~a~%" direction e)
        (sdl2:make-rect 0 0 64 64))))) ; safe fallback

(defclass zone ()
  ((color :initarg :color :accessor zone-color)))

(defclass position ()
  ((x :initarg :x :accessor pos-x)
   (y :initarg :y :accessor pos-y)))

(defclass entity ()
  ((position :initarg :position :accessor entity-position)
   (zone :initarg :zone :accessor entity-zone)
   (color :initarg :color :accessor entity-color)
   (active :initform t :accessor entity-active-p)))

(defclass tank (entity)
  ((mass :initform 10.0 :accessor tank-mass)
   (bullets :initform nil :accessor tank-bullets)
   (direction :initform :north :accessor tank-direction)))

(defclass bullet ()
  ((position :initarg :position :accessor bullet-position)))

(defparameter *global-bullets* '())

(defun safe-make-rect (x y w h)
  "Safely create an SDL2 rectangle with validated parameters"
  (let ((safe-x (safe-clamp (safe-floor x) -32768 32767))
        (safe-y (safe-clamp (safe-floor y) -32768 32767))
        (safe-w (safe-clamp (safe-floor w) 0 32767))
        (safe-h (safe-clamp (safe-floor h) 0 32767)))
    (handler-case
        (sdl2:make-rect safe-x safe-y safe-w safe-h)
      (error (e)
        (format t "Error creating rect (~a,~a,~a,~a): ~a~%" safe-x safe-y safe-w safe-h e)
        (sdl2:make-rect 0 0 1 1))))) ; fallback rect

(defun draw-cell (renderer x y r g b)
  (when (and (integerp x) (integerp y)
             (<= 0 x (- *grid-size* 1))
             (<= 0 y (- *grid-size* 1)))
    (let ((sx (* (- x *camera-x*) *cell-size*))
          (sy (* (- y *camera-y*) *cell-size*)))
      (when (and (safe-number-p sx) (safe-number-p sy)
                 (>= sx -1000) (>= sy -1000)  ; reasonable bounds
                 (< sx (+ *window-width* 1000)) (< sy (+ *window-height* 1000)))
        (handler-case
            (let ((rect (safe-make-rect sx sy *cell-size* *cell-size*)))
              (sdl2:set-render-draw-color renderer 
                                          (safe-clamp r 0 255)
                                          (safe-clamp g 0 255) 
                                          (safe-clamp b 0 255) 255)
              (sdl2:render-fill-rect renderer rect))
          (error (e)
            (format t "Error drawing cell at (~a,~a): ~a~%" x y e)))))))

(defun draw-zones (renderer)
  (handler-case
      (dotimes (x *grid-size*)
        (dotimes (y *grid-size*)
          (let ((color (if (< y 15)
                           '(255 255 255) ; white zone
                           '(120 120 120)))) ; grey zone
            (draw-cell renderer x y (first color) (second color) (third color)))))
    (error (e)
      (format t "Error drawing zones: ~a~%" e))))

(defun draw-tank (renderer tank)
  (handler-case
      (let* ((pos (entity-position tank))
             (x (pos-x pos))
             (y (pos-y pos)))
        
        ;; Validate position
        (unless (and (safe-number-p x) (safe-number-p y))
          (format t "Invalid tank position: (~a,~a)~%" x y)
          (return-from draw-tank))
        
        (let* ((screen-x (* (- x *camera-x*) *cell-size*))
               (screen-y (* (- y *camera-y*) *cell-size*))
               (dst-rect (safe-make-rect screen-x screen-y *cell-size* *cell-size*)))
          
          ;; Sprite draw with extensive safety checks
          (when *tank-sprite*
            (handler-case
                (let ((src-rect (get-sprite-rect (tank-direction tank))))
                  (format t "~&Rendering sprite: direction=~a, src-rect=(~a,~a,~a,~a), dst-rect=(~a,~a,~a,~a)~%"
                          (tank-direction tank)
                          (sdl2:rect-x src-rect) (sdl2:rect-y src-rect) 
                          (sdl2:rect-width src-rect) (sdl2:rect-height src-rect)
                          (sdl2:rect-x dst-rect) (sdl2:rect-y dst-rect)
                          (sdl2:rect-width dst-rect) (sdl2:rect-height dst-rect))
                  
                  ;; Validate rectangles before rendering
                  (when (and (>= (sdl2:rect-x src-rect) 0)
                             (>= (sdl2:rect-y src-rect) 0)
                             (> (sdl2:rect-width src-rect) 0)
                             (> (sdl2:rect-height src-rect) 0)
                             (>= (sdl2:rect-x dst-rect) -1000) ; reasonable bounds
                             (>= (sdl2:rect-y dst-rect) -1000)
                             (> (sdl2:rect-width dst-rect) 0)
                             (> (sdl2:rect-height dst-rect) 0))
                    (sdl2:render-copy renderer *tank-sprite*
                                      :source-rect src-rect
                                      :dest-rect dst-rect)))
              (error (e)
                (format t "~&[ERROR] render-copy failed: ~A~%" e)
                ;; Fallback to colored square on sprite error
                (sdl2:set-render-draw-color renderer 255 0 0 255)
                (sdl2:render-fill-rect renderer dst-rect))))
          
          ;; Fallback visual if texture is missing
          (unless *tank-sprite*
            (sdl2:set-render-draw-color renderer 255 0 0 255)
            (sdl2:render-fill-rect renderer dst-rect))
          
          ;; Health bar with extra safety
          (let* ((bar-width *cell-size*)
                 (bar-height 6)
                 (raw-mass (tank-mass tank))
                 (safe-mass (safe-clamp raw-mass 0.0 20.0))
                 (mass-ratio (if (> safe-mass 0) (/ safe-mass 20.0) 0.0))
                 (bar-fill-float (* bar-width mass-ratio))
                 (bar-fill (safe-clamp (safe-floor bar-fill-float) 0 bar-width))
                 (bar-rect (safe-make-rect screen-x (- screen-y 8) bar-fill bar-height)))
            
            (format t "~&Tank mass: ~A, safe-mass: ~A, ratio: ~A, bar-fill: ~A~%" 
                    raw-mass safe-mass mass-ratio bar-fill)
            
            (sdl2:set-render-draw-color renderer 0 255 0 255)
            (sdl2:render-fill-rect renderer bar-rect))))
    (error (e)
      (format t "Error drawing tank: ~a~%" e))))

(defun draw-bullets (renderer)
  (handler-case
      (dolist (b *global-bullets*)
        (let ((p (bullet-position b)))
          (when (and p (typep p 'position))
            (let ((x (pos-x p))
                  (y (pos-y p)))
              (when (and (safe-number-p x) (safe-number-p y))
                (draw-cell renderer x y 255 255 0))))))
    (error (e)
      (format t "Error drawing bullets: ~a~%" e))))

(defun safe-update-camera (new-x new-y)
  "Safely update camera position"
  (when (and (safe-number-p new-x) (safe-number-p new-y))
    (let ((max-camera-x (max 0 (- *grid-size* *window-cols*)))
          (max-camera-y (max 0 (- *grid-size* *window-rows*))))
      (setf *camera-x* (safe-clamp (- new-x 5) 0 max-camera-x))
      (setf *camera-y* (safe-clamp (- new-y 5) 0 max-camera-y)))))

(defun move-tank (tank dx dy)
  (handler-case
      (let* ((pos (entity-position tank))
             (old-x (pos-x pos))
             (old-y (pos-y pos)))
        
        ;; Validate current position
        (unless (and (safe-number-p old-x) (safe-number-p old-y))
          (format t "Invalid tank position during move: (~a,~a)~%" old-x old-y)
          (setf (pos-x pos) 5) ; reset to safe position
          (setf (pos-y pos) 5)
          (return-from move-tank))
        
        (let ((new-x (safe-clamp (+ old-x dx) 0 (- *grid-size* 1)))
              (new-y (safe-clamp (+ old-y dy) 0 (- *grid-size* 1))))
          
          (format t "Tank moving from (~a,~a) to (~a,~a)~%" old-x old-y new-x new-y)
          (setf (pos-x pos) new-x)
          (setf (pos-y pos) new-y)
          (safe-update-camera new-x new-y)
          (check-tank-zone tank)))
    (error (e)
      (format t "Error moving tank: ~a~%" e))))

(defun check-tank-zone (tank)
  (handler-case
      (let* ((pos (entity-position tank))
             (y (pos-y pos)))
        (when (safe-number-p y)
          (if (< y 15)
              (progn
                (setf (entity-active-p tank) nil)
                (setf (entity-color tank) :white))
              (progn
                (setf (entity-active-p tank) t)
                (setf (entity-color tank) :red)))))
    (error (e)
      (format t "Error checking tank zone: ~a~%" e))))

(defun spawn-bullet (tank)
  (handler-case
      (let ((mass (tank-mass tank)))
        (when (and (safe-number-p mass) (> mass 0.5))
          (let* ((tpos (entity-position tank))
                 (tx (pos-x tpos))
                 (ty (pos-y tpos)))
            (when (and (safe-number-p tx) (safe-number-p ty))
              (let ((bpos (make-instance 'position :x tx :y ty)))
                (setf (tank-mass tank) (max 0.0 (- mass 0.5)))
                (push (make-instance 'bullet :position bpos) *global-bullets*))))))
    (error (e)
      (format t "Error spawning bullet: ~a~%" e))))

(defun update-bullets ()
  (handler-case
      (let ((valid-bullets '()))
        (dolist (b *global-bullets*)
          (let* ((p (bullet-position b))
                 (y (pos-y p)))
            (when (safe-number-p y)
              (let ((new-y (+ y 1)))
                (setf (pos-y p) new-y)
                (when (< new-y *grid-size*)
                  (push b valid-bullets))))))
        (setf *global-bullets* (nreverse valid-bullets)))
    (error (e)
      (format t "Error updating bullets: ~a~%" e))))

(defun regenerate-tank (tank)
  (handler-case
      (when (not (entity-active-p tank))
        (let ((current-mass (tank-mass tank)))
          (when (safe-number-p current-mass)
            (setf (tank-mass tank) (min 20.0 (+ current-mass 0.01))))))
    (error (e)
      (format t "Error regenerating tank: ~a~%" e))))

(defun run-game ()
  (handler-case
      (let* ((start-pos (make-instance 'position :x 5 :y 2))
             (start-zone (make-instance 'zone :color :white))
             (tank (make-instance 'tank :position start-pos :zone start-zone :color :white)))
        
        (sdl2:with-init (:everything)
          (sdl2:with-window (win :title "Flying Tank Zones"
                                 :w *window-width*
                                 :h *window-height*)
            (sdl2:with-renderer (renderer win :flags '(:accelerated :presentvsync))
              
              ;; Texture loading
              (format t "~&Attempting to load texture from assets/tank-sprite.png~%")
              (setf *tank-sprite* (load-texture renderer "/home/rsimtakvim/quicklisp/local-projects/flying-tank/assets/tank-sprite.png"))
              (if *tank-sprite*
                  (format t "~&Texture created successfully: ~A~%" *tank-sprite*)
                  (format t "~&[WARNING] Could not load sprite. Tank will be red square.~%"))
              
              ;; Main loop
              (sdl2:with-event-loop (:method :poll)
                (:keyup
                 (:keysym keysym)
                 (when (sdl2:scancode= (sdl2:scancode-value keysym) :scancode-escape)
                   (sdl2:push-event :quit)))
                
                (:keydown
                 (:keysym keysym)
                 (let ((sym (sdl2:scancode-value keysym)))
                   (cond
                     ((or (sdl2:scancode= sym :scancode-w) (sdl2:scancode= sym :scancode-up))
                      (setf (tank-direction tank) :north) (move-tank tank 0 -1))
                     ((or (sdl2:scancode= sym :scancode-s) (sdl2:scancode= sym :scancode-down))
                      (setf (tank-direction tank) :south) (move-tank tank 0 1))
                     ((or (sdl2:scancode= sym :scancode-a) (sdl2:scancode= sym :scancode-left))
                      (setf (tank-direction tank) :west) (move-tank tank -1 0))
                     ((or (sdl2:scancode= sym :scancode-d) (sdl2:scancode= sym :scancode-right))
                      (setf (tank-direction tank) :east) (move-tank tank 1 0))
                     ((sdl2:scancode= sym :scancode-space)
                      (spawn-bullet tank)))))
                
                (:idle
                 ()
                 (handler-case
                     (progn
                       (update-bullets)
                       (regenerate-tank tank)
                       (sdl2:set-render-draw-color renderer 0 0 0 255)
                       (sdl2:render-clear renderer)
                       (draw-zones renderer)
                       (draw-tank renderer tank)
                       (draw-bullets renderer)
                       (sdl2:render-present renderer)
                       (sdl2:delay 100))
                   (error (e)
                     (format t "~&[ERROR] during idle: ~A~%" e))))
                
                (:quit () t))))))
    (error (e)
      (format t "Fatal error in run-game: ~a~%" e))))
